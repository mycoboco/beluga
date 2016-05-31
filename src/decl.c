/*
 *  declaration parsing
 */

#include <stddef.h>        /* NULL, size_t */
#include <stdio.h>         /* sprintf, fprintf, stderr */
#include <string.h>        /* memcpy */
#include <cbl/arena.h>     /* ARENA_ALLOC, ARENA_CALLOC, ARENA_FREE */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_int */

#include "alist.h"
#include "common.h"
#include "dag.h"
#include "err.h"
#include "expr.h"
#include "in.h"
#include "init.h"
#include "ir.h"
#include "lex.h"
#include "main.h"
#include "op.h"
#include "simp.h"
#include "stmt.h"
#include "strg.h"
#include "sym.h"
#include "ty.h"
#include "decl.h"

/* field calculations;
   ASSUMPTION: bit-field is signed or unsigned int */
#define ADD(x, n)         (((x) > TG_LONG_MAX-(n))? (ovf=1, (x)): (x)+(n))
#define CHKOVERFLOW(x, n) ((void)ADD(x, n))
#define BITUNITS(n)       (((n)+TG_CHAR_BIT-1) / TG_CHAR_BIT)
#define BITUNITD(n)       (((n) + TG_CHAR_BIT*ty_unsignedtype->size - 1) /    \
                           (TG_CHAR_BIT*ty_unsignedtype->size) * ty_unsignedtype->size)
#define BIT2BYTE(n)       ((ir_cur->f.little_bit == ir_cur->f.little_endian)?    \
                              BITUNITS(n): BITUNITD(n))

/* maps size, sign, type to array index */
#define SIZEIDX(x) (((x) == LEX_SHORT)? 1: ((x) == LEX_LONG)? 2: 0)
#define SIGNIDX(x) (((x) == LEX_SIGNED)? 1: ((x) == LEX_UNSIGNED)? 2: 0)
#define TYPEIDX(x) (((x) == LEX_CHAR)? 1: ((x) == LEX_INT)? 2: ((x) == LEX_FLOAT)? 3: \
                    ((x) == LEX_DOUBLE)? 4: 0)

/* resets objects related to decl_cfunc */
#define clear_declobj() (decl_retv=decl_cfunc=NULL, decl_callee=NULL, decl_mainfunc=0)

/* sets lex_pos_t * array */
#define SETPOSA(s, c, d, i) (posa[SPEC]=(s), posa[CLS]=(c), posa[DCLR]=(d), posa[ID]=(i))

/* checks if symbol denotes identifier with linkage */
#define LINKEDID(p) ((p)->sclass != LEX_ENUM && (p)->sclass != LEX_TYPEDEF &&     \
                     ((p)->scope == SYM_SGLOBAL || (p)->sclass == LEX_EXTERN))

#define S(p) ((sym_t *)(p))    /* shorthand for cast to sym_t * */
#define T(p) ((ty_t *)(p))     /* shorthand for cast to ty_t * */


enum { SPEC, CLS, DCLR, ID, LEN };    /* used with lex_pos_t * array */
typedef void *node_t;                 /* refers to void * for readability */


sym_t *decl_retv;       /* symbol for struct/union return value */
sym_t *decl_cfunc;      /* function being currently parsed */
node_t *decl_callee;    /* (sym_t) callee of decl_cfunc */
int decl_mainfunc;      /* true if decl_cfunc is main() */


static alist_t **pautovar,    /* pointer to list of auto locals in compound */
               **pregvar;     /* pointer to list of register locals in compound */
static int regcount;          /* # of register objects in function */
static int strunilev;         /* nesting level of struct/union definitions; separate object in
                                 order not to touch unrelated functions */
static int inparam;           /* true while parsing parameters */


/* internal functions referenced forwardly */
static ty_t *enumdcl(void);
static ty_t *structdcl(int);
static ty_t *dclr(ty_t *, const char **, node_t **,        /* sym_t */
                  int, const lex_pos_t *, lex_pos_t *);
static void exitparam(node_t []);    /* sym_t */
static void decl(sym_t *(*)(int, const char *, ty_t *, const lex_pos_t *[], int));


#define INIT(x) &(ty_ ## x ## type)

/*
 *  parses a declaration specifier
 */
static ty_t *specifier(int *sclass, lex_pos_t *pposcls, int *impl)
{
    ty_t *ty = NULL;
    int cls, cons, vol, sign, size, type;
    lex_pos_t possize = { 0, },    /* short or long */
              possign = { 0, },    /* signed or unsigned */
              poscon =  { 0, },    /* qualifier const */
              posvol =  { 0, };    /* qualifier volatile */

    assert(impl);

    *impl = 1;
    cls = cons = vol = sign = size = type = 0;
    while (1) {
        int *p, tt = lex_tc;
        switch(lex_tc) {
            /* storage-class specifier */
            case LEX_AUTO:
            case LEX_REGISTER:
            case LEX_STATIC:
            case LEX_EXTERN:
            case LEX_TYPEDEF:
                if (!cls && (cons || vol || sign || size || type))
                    err_issuep(lex_cpos, ERR_PARSE_CLSFIRST, lex_tc);
                if (!sclass) {
                    err_issuep(lex_cpos, ERR_PARSE_CLS);
                    cls = 0;    /* shuts up ERR_PARSE_INVUSE */
                } else if (!cls && pposcls)
                    *pposcls = *lex_cpos;
                p = &cls;
                lex_tc = lex_next();
                break;
            /* type qualifier */
            case LEX_CONST:
                if (poscon.g.y == 0)
                    poscon = *lex_cpos;
                p = &cons;
                lex_tc = lex_next();
                break;
            case LEX_VOLATILE:
                if (posvol.g.y == 0)
                    posvol = *lex_cpos;
                p = &vol;
                lex_tc = lex_next();
                break;
            /* type specifier */
            case LEX_SIGNED:
            case LEX_UNSIGNED:
                *impl = 0;
                if (possign.g.y == 0)
                    possign = *lex_cpos;
                p = &sign;
                lex_tc = lex_next();
                break;
            case LEX_LONG:
            case LEX_SHORT:
                if (possize.g.y == 0)
                    possize = *lex_cpos;
                p = &size;
                lex_tc = lex_next();
                break;
            case LEX_VOID:
            case LEX_CHAR:
            case LEX_INT:
            case LEX_FLOAT:
            case LEX_DOUBLE:
                p = &type;
                lex_tc = lex_next();
                break;
            case LEX_ENUM:
                p = &type;
                ty = enumdcl();
                break;
            case LEX_STRUCT:
            case LEX_UNION:
                p = &type;
                ty = structdcl(lex_tc);
                break;
            case LEX_ID:
                if (lex_istype() && !type && !sign && !size) {
                    if (main_opt()->xref)
                        sym_use(lex_sym, lex_cpos);
                    ty = lex_sym->type;
                    *impl = ty->t.plain;
                    p = &type;
                    lex_tc = lex_next();
                } else
                    p = NULL;
                break;
            default:
                p = NULL;
                break;
        }
        if (!p)
            break;
        if (*p) {
            if (p == &cons || p == &vol)
                err_issuep(lex_ppos, ERR_TYPE_DUPQUAL, tt);
            else
                err_issuep(lex_ppos, ERR_PARSE_INVUSE, tt);
        } else
            *p = tt;
    }
    if (sclass)
        *sclass = cls;
    {
        static ty_t **tab[3][3][5] = {    /* none/short/long * none/signed/unsigned * v/c/i/f/d */
            /* redundant parentheses indicate illegal combinations */
            {    /* none */
 /* none */     { INIT(void),   INIT(char),    INIT(int),      INIT(float),    INIT(double)   },
 /* signed */   { (INIT(void)), INIT(schar),   INIT(int),      (INIT(float)),  (INIT(double)) },
 /* unsigned */ { (INIT(void)), INIT(uchar),   INIT(unsigned), (INIT(float)),  (INIT(double)) }
            },
            {    /* short */
 /* none */     { (INIT(void)), (INIT(char)),  INIT(short),    (INIT(float)),  (INIT(float)) },
 /* signed */   { (INIT(void)), (INIT(schar)), INIT(short),    (INIT(float)),  (INIT(float)) },
 /* unsigned */ { (INIT(void)), (INIT(uchar)), INIT(ushort),   (INIT(float)),  (INIT(float)) }
            },
            {    /* long */
 /* none */     { (INIT(void)), (INIT(char)),  INIT(long),     (INIT(double)), INIT(ldouble)   },
 /* signed */   { (INIT(void)), (INIT(schar)), INIT(long),     (INIT(double)), (INIT(ldouble)) },
 /* unsigned */ { (INIT(void)), (INIT(uchar)), INIT(ulong),    (INIT(double)), (INIT(ldouble)) }
            }
        };

        if (!type) {
            if (!size && !sign) {
                err_issuep(lex_cpos, ERR_PARSE_DEFINT);
                err_issuep(lex_cpos, ERR_PARSE_DEFINTSTD);
                *impl |= (1 << 1);
            }
            type = LEX_INT;
        }
        if ((size == LEX_SHORT && type != LEX_INT) ||
            (size == LEX_LONG && type != LEX_INT && type != LEX_DOUBLE))
            err_issuep(&possize, ERR_PARSE_INVTYPE, size, type);
        else if (sign && type != LEX_INT && type != LEX_CHAR)
            err_issuep(&possign, ERR_PARSE_INVTYPE, sign, type);
        if (!ty) {
            ty = *tab[SIZEIDX(size)][SIGNIDX(sign)][TYPEIDX(type)];
            assert(ty);
        }
        if (cons) {
            err_entersite(&poscon);    /* enters with const */
            ty = ty_qual_s(TY_CONST, ty, 0);
            err_exitsite();    /* exits from const */
        }
        if (vol) {
            err_entersite(&posvol);    /* enters with volatile */
            ty = ty_qual_s(TY_VOLATILE, ty, 0);
            err_exitsite();    /* exits from volatile */
        }
    }

    return ty;
}

#undef INIT


/*
 *  checks the length of a declared identifier
 */
int (decl_chkid)(const char *id, const lex_pos_t *ppos, sym_tab_t *tp, int glb)
{
    sym_t *p;

    assert(id);
    assert(ppos);
    assert(tp);

    if ((p = sym_clookup(id, tp, glb)) != NULL && (!p->type || !TY_ISUNKNOWN(p->type))) {
        err_issuep(ppos, (glb)? ERR_LEX_LONGEID: ERR_LEX_LONGID, p->name, &p->pos);
        err_issuep(ppos, ERR_LEX_LONGIDSTD, (long)((glb)? TL_ENAME_STD: TL_INAME_STD));
        return 1;
    }

    return 0;
}


/*
 *  parses struct/union fields;
 *  ASSUMPTION: alignment factors must be a power of 2;
 *  ASSUMPTION: a bit-field cannot straddle two storage units
 */
static int field(ty_t *ty)
{
    int unknown = 0;
    int inparamp = inparam;
    lex_pos_t posdclr,     /* declarator */
              posbfld,     /* bitfield expression */
              poscolon,    /* ':' for bitfield */
              posid;       /* identifier */

    assert(ty);
    assert(ty_inttype);    /* ensures types initialized */
    assert(ir_cur);

    inparam = 0;
    {
        int plain;
        int n = 0;

        while (lex_issdecl()) {
            ty_t *ty1 = specifier(NULL, NULL, &plain);
            while (1) {
                sym_field_t *p;
                const char *id = NULL;
                ty_t *fty;

                if (lex_isadcl() || lex_tc == ':' || lex_tc == ';')    /* ; for extension */
                    posdclr = *lex_cpos;
                else {
                    posdclr.g.f = NULL;
                    err_issuep(lex_epos(), ERR_PARSE_NODCLR, " for member");
                }
                if (posdclr.g.f) {
                    fty = dclr(ty1, &id, NULL, 0, &posdclr, &posid);
                    unknown |= fty->t.unknown;
                    err_entersite(&posdclr);    /* enters with declarator */
                    p = ty_newfield_s(id, ty, fty);
                    err_exitsite();    /* exits from declarator */
                    if (!ty_hasproto(p->type))
                        err_issuep(&posdclr, ERR_PARSE_NOPROTO);
                    if (lex_tc == ':') {
                        poscolon = *lex_cpos;
                        p->type = p->type->t.type;
                        if (!TY_ISINT(p->type) && !TY_ISUNSIGNED(p->type)) {
                            if (!TY_ISUNKNOWN(p->type))
                                err_issuep(&poscolon, ERR_PARSE_INVBITTYPE);
                            p->type = ty_inttype;
                            plain = 0;    /* shuts up additional warnings */
                        }
                        lex_tc = lex_next();
                        posbfld = *lex_cpos;
                        p->bitsize = 1, simp_intexpr(0, &p->bitsize, 0, "bit-field size");
                        if (p->bitsize > TG_CHAR_BIT*p->type->size || p->bitsize < 0) {
                            err_issuep(&posbfld, ERR_PARSE_INVBITSIZE,
                                       (long)TG_CHAR_BIT*p->type->size);
                            p->bitsize = TG_CHAR_BIT*p->type->size;
                        } else if (p->bitsize == 0 && id) {
                            err_issuep(&posdclr, ERR_PARSE_EXTRAID, id, "");
                            p->name = hash_int(sym_genlab(1));
                        } else if ((plain & 1) && id) {
                            err_issuep(&poscolon, ERR_PARSE_PINTFLD);
                            p->type = ty_qualc(p->type->op & TY_CONSVOL, (main_opt()->pfldunsign)?
                                                ty_unsignedtype:
                                                ty_inttype);    /* always succeeds */
                        }
                        p->lsb = 1;
                    } else if ((main_opt()->std == 0 || main_opt()->std > 2) && !id &&
                               TY_ISSTRUNI(p->type)) {
                        if (!GENNAME(TY_UNQUAL(p->type)->u.sym->name))
                            err_issuep(&(TY_UNQUAL(p->type)->u.sym->pos), ERR_PARSE_ANONYTAG);
                        if (p->type->size == 0)
                            err_issuep(&posdclr, ERR_PARSE_INCOMPMEM);
                        p->name = NULL;
                    } else {
                        if (!id)
                            err_issuep(&posdclr, ERR_PARSE_NOFNAME);
                        if (TY_ISFUNC(p->type))
                            err_issuep(&posdclr, ERR_PARSE_INVFTYPE);
                        else if (p->type->size == 0) {
                            p->incomp = 1;
                            err_issuep(&posdclr, ERR_PARSE_INCOMPMEM);
                        }
                    }
                    if (TY_ISCONST(ty_arrelem(p->type)) || TY_HASCONST(p->type))
                        ty->u.sym->u.s.cfield = 1;
                    if (TY_ISVOLATILE(ty_arrelem(p->type)) || TY_HASVOLATILE(p->type))
                        ty->u.sym->u.s.vfield = 1;
                    if (n++ == TL_MBR_STD) {
                        err_issuep(&posdclr, ERR_PARSE_MANYMBR);
                        err_issuep(&posdclr, ERR_PARSE_MANYMBRSTD, (long)TL_MBR_STD);
                    }
                }
                if (lex_tc != ',')
                    break;
                lex_tc = lex_next();
                if (lex_extracomma(';', "member declarator", 0))
                    break;
            }
            err_test(';', err_sset_field);
        }
    }
    {
        long off = 0;
        int bit = 0, ovf = 0;
        sym_field_t *p, **q = &ty->u.sym->u.s.flist;
        ty->align = ir_cur->structmetric.align;

        for (p = *q; p; p = p->link) {
            int a = (p->type->align)? p->type->align: 1;
            if (p->lsb)
                assert(p->type->align == ty_unsignedtype->align);
            if (ty->op == TY_UNION)
                off = bit = 0;
            else if (p->bitsize == 0 || bit == 0 ||
                     bit-1+p->bitsize > TG_CHAR_BIT*p->type->size) {
                off = ADD(off, BIT2BYTE(bit-1));
                bit = 0;
                CHKOVERFLOW(off, a-1);
                off = ROUNDUP(off, a);
            }
            if (a > ty->align)
                ty->align = a;
            p->offset = off;
            if (p->lsb) {
                if (bit == 0)
                    bit = 1;
                if (ir_cur->f.little_bit)
                    p->lsb = bit;
                else
                    p->lsb = TG_CHAR_BIT*p->type->size - bit + 1 - p->bitsize + 1;
                bit += p->bitsize;
            } else
                off = ADD(off, p->type->size);
            if (off + BIT2BYTE(bit-1) > ty->size)
                ty->size = off + BIT2BYTE(bit-1);
            if (!p->name || !GENNAME(p->name)) {
                *q = p;
                q = &p->link;
            }
        }
        *q = NULL;
        CHKOVERFLOW(ty->size, ty->align-1);
        ty->size = ROUNDUP(ty->size, ty->align);
        if (ovf) {
            ty->size = TG_LONG_MAX & (~(ty->align - 1));
            err_issuep(&ty->u.sym->pos, ERR_TYPE_BIGOBJADJ, ty->size);
        }
        if (strunilev == 1 && (main_opt()->std == 0 || main_opt()->std > 2))
            ty_chkfield(ty);
    }
    inparam = inparamp;
    assert(inparam >= 0);

    return unknown;
}


/*
 *  parses a struct/union specifier
 */
static ty_t *structdcl(int op)
{
    ty_t *ty;
    const char *tag;

    lex_tc = lex_next();
    err_entersite(lex_cpos);    /* enters with tag */
    if (lex_tc == LEX_ID) {
        tag = lex_tok;
        lex_tc = lex_next();
    } else
        tag = "";
    if (lex_tc == '{') {
        if (*tag == '\0' && inparam)
            err_issuep(lex_epos(), ERR_PARSE_ATAGPARAM, op);
        if (strunilev++ == TL_STRCT_STD) {
            err_issuep(lex_cpos, ERR_PARSE_MANYSTR);
            err_issuep(lex_cpos, ERR_PARSE_MANYSTRSTD, (long)TL_STRCT_STD);
        }
        ty = ty_newstruct_s(lex_tc, op, tag);
        ty->u.sym->pos = *err_getppos();
        ty->u.sym->f.defined = 1;
        lex_tc = lex_next();
        if (lex_issdecl())
            ty->t.unknown = field(ty);
        else if (lex_tc == '}')
            err_issuep(lex_epos(), ERR_PARSE_NOFIELD, op);
        else
            err_issuep(lex_cpos, ERR_PARSE_INVFIELD, op);
        err_test('}', err_sset_strdef);
        {    /* fixes size of typedef name completed after its definition */
            size_t n;
            alist_t *pos;
            ALIST_FOREACH(n, pos, ty->u.sym->u.s.blist) {
                S(pos->data)->type->size = ty->size;
                S(pos->data)->type->align = ty->align;
            }
        }
        strunilev--;
        assert(strunilev >= 0);
    } else {
        if (*tag == '\0')
            err_issuep(lex_epos(), ERR_PARSE_NOTAG, op);
        ty = ty_newstruct_s(lex_tc, op, tag);
    }
    if (*tag && main_opt()->xref)
        sym_use(ty->u.sym, err_getppos());

    err_exitsite();    /* exit from tag */
    return ty;
}


/*
 *  parses an enum specifier;
 *  ASSUMPTION: LONG_MAX >= TG_INT_MAX
 */
static ty_t *enumdcl(void)
{
    const char *tag;
    ty_t *ty;

    assert(ty_inttype);    /* ensures types initialized */

    lex_tc = lex_next();
    err_entersite(lex_cpos);    /* enters with tag */
    if (lex_tc == LEX_ID) {
        tag = lex_tok;
        lex_tc = lex_next();
    } else
        tag = "";

    if (lex_tc == '{') {
        int n = 0;
        long k = -1;
        alist_t *idlist = NULL;
        ty = ty_newstruct_s(lex_tc, TY_ENUM, tag);

        lex_tc = lex_next();
        if (lex_tc != LEX_ID)
            err_issuep(lex_epos(), ERR_PARSE_ENUMID);
        else
            do {
                sym_t *p;
                const char *id = lex_tok;
                lex_pos_t posenum = *lex_cpos;    /* enumerator */
                if (lex_sym) {
                    if (!TY_ISUNKNOWN(lex_sym->type))
                        err_issuep(&posenum, (SYM_SAMESCP(lex_sym, sym_scope))?
                                                 ERR_PARSE_REDECL1: ERR_PARSE_HIDEID,
                                   lex_sym, " an identifier", &lex_sym->pos);
                } else
                    decl_chkid(id, &posenum, sym_ident, 0);
                lex_tc = lex_next();
                if (lex_tc == '=') {
                    lex_tc = lex_next();
                    k = 0, simp_intexpr(0, &k, 1, "enum constant");
                } else {
                    if (k == TG_INT_MAX) {
                        err_issuep(lex_ppos, ERR_PARSE_ENUMOVER, id, "");
                        k = -1;
                    }
                    k++;
                }
                p = sym_new(SYM_KENUM, id, &posenum, ty,
                            (sym_scope < SYM_SPARAM)? strg_perm: strg_func, k);
                idlist = alist_append(idlist, p, strg_perm);
                if (n++ == TL_ENUMC_STD) {
                    err_issuep(lex_ppos, ERR_PARSE_MANYEC);
                    err_issuep(lex_ppos, ERR_PARSE_MANYECSTD, (long)TL_ENUMC_STD);
                }
                if (lex_tc != ',')
                    break;
                lex_tc = lex_next();
                if (lex_tc == '}')
                    err_issuep(lex_ppos, ERR_PARSE_ENUMCOMMA);
                else if (lex_extracomma('}', "enumerator", 1))
                    break;
            } while(lex_tc == LEX_ID);
        if (lex_tc == ';') {
            err_issuep(lex_cpos, ERR_PARSE_ENUMSEMIC);
            lex_tc = lex_next();
        }
        err_test('}', err_sset_enumdef);
        ty->u.sym->u.idlist = alist_toarray(idlist, strg_perm);
        ty->u.sym->pos = *err_getppos();
        ty->u.sym->f.defined = 1;
    } else {
        if (*tag == '\0')
            err_issuep(lex_epos(), ERR_PARSE_NOTAG, TY_ENUM);
        ty = ty_newstruct_s(lex_tc, TY_ENUM, tag);
        if (lex_tc == ';')
            err_issuep(lex_epos(), ERR_PARSE_EMPTYDECL);
    }
    ty->type = ty_inttype;
    ty->size = ty->type->size;
    ty->align = ty->type->align;
    if (*tag && main_opt()->xref)
        sym_use(ty->u.sym, err_getppos());

    err_exitsite();    /* exit from tag */
    return ty;
}


/*
 *  skips erroneous initializers
 */
static void skipinit(const char *msg)
{
    if (lex_tc != '=')
        return;
    if (msg)
        err_issuep(lex_cpos, ERR_PARSE_NOINIT, msg);
    lex_tc = lex_next();
    init_skip();
}


/*
 *  installs a parameter
 */
static sym_t *dclparam(int sclass, const char *id, ty_t *ty, const lex_pos_t *posa[], int n)
{
    sym_t *p;

    assert(id);
    assert(ty);
    assert(posa[SPEC]);
    assert(posa[DCLR]);

    if (TY_ISFUNC(ty))
        ty = ty_ptr(ty);
    else if (TY_ISARRAY(ty)) {
        err_entersite(posa[DCLR]);    /* enters with declarator */
        ty = ty_atop_s(ty);
        err_exitsite();    /* exits from declarator */
    }

    if (sclass > 0 && sclass != LEX_REGISTER) {
        assert(posa[CLS]);
        if (n == 0)
            err_issuep(posa[CLS], ERR_PARSE_INVCLS, sclass);
        sclass = LEX_AUTO;
    }

    p = sym_lookup(id, sym_ident);
    if (p) {
        if (!TY_ISUNKNOWN(p->type)) {
            if (p->scope == sym_scope) {
                err_issuep(posa[DCLR], ERR_PARSE_REDECL1, p, " an identifier", &p->pos);
                id = sym_semigenlab();    /* avoids type change of existing param */
            } else if (sclass != -1 && !GENSYM(p))
                err_issuep(posa[DCLR], ERR_PARSE_HIDEID, p, " an identifier", &p->pos);
        }
    } else if (sclass != -1)
        decl_chkid(id, posa[DCLR], sym_ident, 0);
    if (sclass <= 0)
        sclass = LEX_AUTO;
    p = sym_new(SYM_KORDIN, id, posa[DCLR], sclass, ty, strg_func);
    p->f.wregister = (sclass == LEX_REGISTER);
    p->f.defined = 1;
    skipinit("parameter");

    return p;
}


#define ISSUEONCE(m, p)                      \
    do {                                     \
        if (!m)                              \
            err_issuep(p, ERR_PARSE_##m);    \
        m = 1;                               \
    } while(0)

/*
 *  parses parameters
 */
static node_t *parameter(ty_t *fty)    /* sym_t */
{
    alist_t *list = NULL;
    node_t *param;    /* sym_t */
    lex_pos_t posspec,     /* declaration specifier */
              posfspec,    /* first declaration specifier */
              poscls,      /* storage-class specifier */
              posdclr,     /* declarator */
              posell,      /* '...' if any */
              posid;       /* identifier */
    const lex_pos_t *posa[LEN];

    assert(fty);
    assert(ty_voidtype);    /* ensures types initialized */

    SETPOSA(&posspec, NULL, &posdclr, NULL);
    sym_enterscope();
    if (sym_scope > SYM_SPARAM)
        sym_enterscope();

    inparam++;
    while (1)    /* to handle mixed proto/non-proto */
        if (lex_isparam() || lex_tc == LEX_ELLIPSIS) {
            int n = 0;
            ty_t *ty1 = NULL;
            int ellipsis = 0;    /* ellipsis seen */
            int VOIDALONE,    /* ERR_PARSE_VOIDALONE issued */
                ELLALONE,     /* ERR_PARSE_ELLALONE issued */
                ELLSEEN,      /* ERR_PARSE_ELLSEEN issued */
                QUALVOID;     /* ERR_PARSE_QUALVOID issued */
            VOIDALONE = ELLALONE = ELLSEEN = QUALVOID = 0;
            while (1) {
                ty_t *ty;
                int sclass = 0;
                const char *id = NULL;
                if (lex_tc == LEX_ELLIPSIS) {
                    if (ty1 && !ellipsis) {
                        static sym_t sentinel;
                        posell = *lex_cpos;
                        if (!sentinel.type)
                            sentinel.type = ty_voidtype;
                        if (ty1 == ty_voidtype)
                            ISSUEONCE(VOIDALONE, &posfspec);
                        else {
                            list = alist_append(list, &sentinel, strg_perm);
                            ellipsis = 1;
                        }
                    } else if (ellipsis)
                        ISSUEONCE(ELLSEEN, &posell);
                    else    /* !ty1 */
                        ISSUEONCE(ELLALONE, lex_cpos);
                    lex_tc = lex_next();
                } else {
                    int dummy;
                    int strunilevp = strunilev;
                    n++;
                    if (!lex_isparam())
                        err_issuep(lex_epos(), ERR_PARSE_NOPTYPE);
                    strunilev = 0;
                    posspec = *lex_cpos;
                    ty = specifier(&sclass, &poscls, &dummy);
                    strunilev = strunilevp;
                    if (sclass)
                        posa[CLS] = &poscls;
                    posdclr = *lex_cpos;
                    ty = dclr(ty, &id, NULL, 1, &posdclr, &posid);
                    if (TY_ISVOID(ty) || ty1 == ty_voidtype) {
                        if (TY_ISVOID(ty) && id)
                            err_issuep(&posspec, ERR_PARSE_VOIDID);
                        if (!ty1 && TY_ISQUAL(ty) && TY_ISVOID(ty) && !id)
                            ISSUEONCE(QUALVOID, &posspec);
                        if (ty1)
                            ISSUEONCE(VOIDALONE, &posspec);
                    }
                    if (ellipsis)
                        ISSUEONCE(ELLSEEN, &posell);
                    if (ty1 != ty_voidtype && !TY_ISVOID(ty) && !ellipsis) {
                        if (!id)
                            id = hash_int(n);
                        list = alist_append(list, dclparam(sclass, id, ty, posa, 0), strg_perm);
                        if (!ty_hasproto(ty))
                            err_issuep(&posdclr, ERR_PARSE_NOPROTO);
                    } else
                        skipinit("parameter");
                    if (!ty1) {
                        ty1 = TY_UNQUAL(ty)->t.type;
                        posfspec = posspec;
                    }
                }
                if (lex_tc != ',')
                    break;
                lex_tc = lex_next();
                if (lex_extracomma(')', "parameter declaration", 0))
                    break;
            }
            fty->u.f.proto = ARENA_ALLOC(strg_perm,
                                         sizeof(*fty->u.f.proto)*(alist_length(list)+1));
            param = alist_toarray(list, strg_func);
            for (n = 0; param[n]; n++)
                fty->u.f.proto[n] = S(param[n])->type;
            fty->u.f.proto[n] = NULL;
            fty->u.f.oldstyle = 0;
            break;
        } else {
            if (lex_tc == LEX_ID || lex_tc == '=')    /* accepts erroneous initializer */
                while (1) {
                    sym_t *p;
                    posspec = posdclr = *lex_cpos;
                    if (lex_tc == LEX_ID) {
                        p = dclparam(-1, lex_tok, ty_inttype, posa, 0);
                        p->f.defined = 0;    /* old-style */
                        list = alist_append(list, p, strg_perm);
                        lex_tc = lex_next();
                    } else
                        err_issuep(lex_cpos, ERR_PARSE_PARAMID);
                    skipinit("parameter");
                    if (lex_tc != ',')
                        break;
                    lex_tc = lex_next();
                    if (lex_extracomma(')', "parameter identifier", 0))
                        break;
                }
            if (lex_isparam() || lex_tc == LEX_ELLIPSIS) {
                err_issuep(lex_cpos, ERR_PARSE_MIXPROTO);
                list = NULL;
                continue;
            }
            param = alist_toarray(list, strg_func);
            fty->u.f.proto = NULL;
            fty->u.f.oldstyle = 1;
            break;
        }
    err_test(')', err_sset_initb);
    inparam--;
    assert(inparam >= 0);

    return param;
}

#undef ISSUEONCE


/*
 *  constructs a temporary type node for dclr1()
 */
static ty_t *tnode(int op, ty_t *type)
{
    ty_t *ty;

    ty = ARENA_CALLOC(strg_stmt, sizeof(*ty), 1);
    ty->op = op;
    ty->type = type;

    return ty;
}


/*
 *  constructs an inverted type from a declarator
 */
static ty_t *dclr1(const char **id, node_t **param,             /* sym_t */
                   int abstract, int lev, lex_pos_t *pposid)
{
    ty_t *ty = NULL;

    switch(lex_tc) {
        case LEX_ID:
            if (pposid)
                *pposid = *lex_cpos;
            if (!id)
                err_issuep(lex_cpos, ERR_PARSE_EXTRAID, lex_tok, "");
            else
                *id = lex_tok;
            lex_tc = lex_next();
            break;
        case '*':
            lex_tc = lex_next();
            if (lex_tc == LEX_CONST || lex_tc == LEX_VOLATILE) {
                ty_t *ty1;
                ty1 = ty = tnode(lex_tc, NULL);
                while ((lex_tc = lex_next()) == LEX_CONST || lex_tc == LEX_VOLATILE)
                    ty1 = tnode(lex_tc, ty1);
                ty->type = dclr1(id, param, abstract, lev, pposid);
                ty = ty1;
            } else
                ty = dclr1(id, param, abstract, lev, pposid);
            ty = tnode(TY_POINTER, ty);
            break;
        case '(':
            lex_tc = lex_next();
            if (abstract && (lex_isparam() || lex_tc == ')')) {
                node_t *arg;    /* sym_t */
                ty = tnode(TY_FUNCTION, ty);
                arg = parameter(ty);
                exitparam(arg);
            } else {
                if (!(lex_tc == LEX_ID || lex_tc == '*' || lex_tc == '('))
                    goto fparam;
                if (lev == TL_PAREND_STD) {
                    err_issuep(lex_ppos, ERR_PARSE_MANYPD);
                    err_issuep(lex_ppos, ERR_PARSE_MANYPDSTD, (long)TL_PAREND_STD);
                }
                ty = dclr1(id, param, abstract, lev+1, pposid);
                err_expect(')');
                if (abstract && !ty && (!id || !*id))
                    return tnode(TY_FUNCTION, NULL);
            }
            break;
        default:
            break;
    }
    while (lex_tc == '(' || lex_tc == '[')
        switch(lex_tc) {
            case '(':
                lex_tc = lex_next();
            fparam:
                {
                    node_t *arg;    /* sym_t */
                    ty = tnode(TY_FUNCTION, ty);
                    arg = parameter(ty);
                    if (param && !*param)
                        *param = arg;
                    else
                        exitparam(arg);
                }
                break;
            case '[':
                lex_tc = lex_next();
                {
                    long n = 0;
                    lex_pos_t posexpr;    /* size expression */
                    if (lex_isexpr()) {
                        posexpr = *lex_cpos;
                        n = 1, simp_intexpr(']', &n, 1, "array size");
                        if (n <= 0) {
                            err_issuep(&posexpr, ERR_PARSE_INVARRSIZE);
                            n = 1;
                        }
                    } else
                        err_expect(']');
                    ty = tnode(TY_ARRAY, ty);
                    ty->size = n;
                }
                break;
            default:
                assert(!"impossible case -- should never reach here");
                break;
        }

    return ty;
}


/*
 *  constructs a normal type from an inverted type
 */
static ty_t *dclr(ty_t *basety, const char **id, node_t **param,                 /* sym_t */
                  int abstract, const lex_pos_t *pposdclr, lex_pos_t *pposid)
{
    int n = 0;
    ty_t *ty;

    assert(basety);
    assert(pposdclr);

    err_entersite(pposdclr);    /* enters with declarator */
    for (ty = dclr1(id, param, abstract, 0, pposid); ty; ty = ty->type)
        switch(ty->op) {
            case TY_POINTER:
                basety = ty_ptr(basety);
                n++;
                break;
            case TY_FUNCTION:
                basety = ty_func_s(basety, ty->u.f.proto, ty->u.f.oldstyle);
                n++;
                break;
            case TY_ARRAY:
                basety = ty_array_s(basety, ty->size);
                n++;
                break;
            case TY_CONST:
            case TY_VOLATILE:
                basety = ty_qual_s(ty->op, basety, 1);
                break;
            default:
                assert(!"invalid type operator -- should never reach here");
                break;
        }
    if (n > TL_DECL_STD) {
        err_issue_s(ERR_PARSE_MANYDECL);
        err_issue_s(ERR_PARSE_MANYDECLSTD, (long)TL_DECL_STD);
    }
    if (basety->size > TL_OBJ_STD) {    /* note TL_OBJ_STD is of unsigned long */
        err_issue_s(ERR_TYPE_BIGOBJ);
        err_issue_s(ERR_TYPE_BIGARRSTD, (unsigned long)TL_OBJ_STD);
    }

    err_exitsite();    /* exits from declarator */
    return basety;
}


/*
 *  makes the back-end define a global
 */
void (decl_defglobal)(sym_t *p, int seg)
{
    assert(p);
    assert(ir_cur);

    p->u.seg = seg;
    init_swtoseg(p->u.seg);
    if (p->sclass != LEX_STATIC)
        ir_cur->export(p);
    ir_cur->defglobal(p);
}


/*
 *  recognizes an initializer and defines a global object
 */
static void initglobal(sym_t *p, const lex_pos_t *ppos, int tolit)
{
    ty_t *ty;

    assert(p);
    assert(ppos);
    assert(lex_tc != '=');

    err_entersite(ppos);    /* enters with = */
    if (p->sclass == LEX_STATIC) {
        ty = ty_arrelem(p->type);
        decl_defglobal(p, (tolit || TY_ISCONST(ty))? INIT_SEGLIT: INIT_SEGDATA);
    } else
        decl_defglobal(p, INIT_SEGDATA);
    ty = init_init_s(p->type, 0);
    if (TY_ISARRAY(p->type) && p->type->size == 0) {
        p->type = ty;
        if (p->type->size > 0)
            ir_cur->cmpglobal(p);
        else
            assert(err_count() > 0);
    }
    if (p->sclass == LEX_EXTERN)
        p->sclass = LEX_AUTO;
    p->f.defined = 1;
    err_exitsite();    /* exits from = */
}


/*
 *  installs a typedef name
 */
static sym_t *typedefsym(const char *id, ty_t *ty, const lex_pos_t *pposdclr, int plain)
{
    sym_t *p;

    assert(id);
    assert(ty);
    assert(pposdclr);

    p = sym_lookup(id, sym_ident);
    if (p) {
        if (!TY_ISUNKNOWN(p->type))
            err_issuep(pposdclr, (SYM_SAMESCP(p, sym_scope))? ERR_PARSE_REDECL1: ERR_PARSE_HIDEID,
                       p, " an identifier", &p->pos);
    } else
        decl_chkid(id, pposdclr, sym_ident, 0);
    ty = memcpy(ARENA_ALLOC(strg_perm, sizeof(*ty)), ty, sizeof(*ty));
    ty->t.name = id;
    ty->t.plain = plain;
    p = sym_new(SYM_KTYPEDEF, id, pposdclr, ty, (sym_scope < SYM_SPARAM)? strg_perm: strg_func);
    if (TY_ISSTRUNI(ty) && ty->size == 0) {
        alist_t **pl = &(TY_UNQUAL(ty)->u.sym->u.s.blist);
        *pl = alist_append(*pl, p, strg_perm);
    }
    skipinit("type name");

    return p;
}


/*
 *  compares a type against type list entires
 */
static void cmptylist(sym_t *p, sym_t *q)
{
    int eqret;
    sym_tylist_t *pt;

    assert(p);
    assert(q);

    for (pt = q->tylist; pt; pt = pt->next) {
        if ((eqret = ty_equiv(p->type, pt->type, 1)) != 0) {
            if (eqret > 1)
                err_issuep(&p->pos, ERR_PARSE_ENUMINT, &pt->pos);
        } else
            err_issuep(&p->pos, ERR_PARSE_REDECL2, p, " an identifier", &pt->pos);
    }
    if (ty_equiv(p->type, q->type, 1)) {
        q->type = ty_compose(p->type, q->type);
        q->pos = p->pos;
    }
    q->tylist = sym_tylist(q->tylist, p);
}


/*
 *  installs a global identifier
 */
static sym_t *dclglobal(int sclass, const char *id, ty_t *ty, const lex_pos_t *posa[], int n)
{
    sym_t *p;
    int visible = 0, eqret;

    assert(sclass != LEX_TYPEDEF);
    assert(id);
    assert(!GENNAME(id));
    assert(ty);
    assert(posa[SPEC]);
    assert(posa[DCLR]);
    assert(ir_cur);

    if (sclass && sclass != LEX_EXTERN && sclass != LEX_STATIC) {
        assert(posa[CLS]);
        if (n == 0)
            err_issuep(posa[CLS], ERR_PARSE_INVCLS, sclass);
        sclass = 0;
    }
    if (!sclass)
        sclass = (TY_ISFUNC(ty))? LEX_EXTERN: LEX_AUTO;
    p = sym_lookup(id, sym_global);
    if (p) {
        visible = 1;
        if (p->sclass != LEX_TYPEDEF && p->sclass != LEX_ENUM &&
            (eqret = ty_equiv(ty, p->type, 1)) != 0) {
            if (eqret > 1)
                err_issuep(posa[DCLR], ERR_PARSE_ENUMINT, &p->pos);
            ty = ty_compose(ty, p->type);
        } else if (!TY_ISUNKNOWN(p->type)) {
            assert(sym_scope == SYM_SGLOBAL || sym_scope == SYM_SPARAM);
            err_issuep(posa[DCLR], ERR_PARSE_REDECL1, p, " an identifier", &p->pos);
        }
        if (!TY_ISFUNC(ty) && p->f.defined && lex_tc == '=' && eqret)
            err_issuep(lex_cpos, ERR_PARSE_REDEF, p, " an identifier", &p->pos);
        if ((p->sclass == LEX_EXTERN && sclass == LEX_STATIC) ||
            (p->sclass == LEX_STATIC && sclass == LEX_AUTO) ||
            (p->sclass == LEX_AUTO && sclass == LEX_STATIC))
            err_issuep(posa[(posa[CLS])? CLS: SPEC], ERR_PARSE_INVLINK, p, " an identifier",
                       &p->pos);
        if (p->sclass == LEX_EXTERN ||
            p->sclass == LEX_TYPEDEF)    /* to pass to symgsc() */
            p->sclass = sclass;
    } else {
        p = sym_new(SYM_KGLOBAL, id, posa[DCLR], sclass, ty);
        if (sclass != LEX_STATIC) {
            static int nglobal;
            if (nglobal++ == TL_NAME_STD) {
                err_issuep(posa[DCLR], ERR_PARSE_MANYEID);
                err_issuep(posa[DCLR], ERR_PARSE_MANYEIDSTD, (long)TL_NAME_STD);
            }
            if (!decl_chkid(id, posa[ID], sym_global, 1))
                decl_chkid(id, posa[ID], sym_extern, 1);
        } else
            decl_chkid(id, posa[ID], sym_ident, 0);
    }
    p->type = ty;
    p->pos = *posa[DCLR];
    ir_cur->symgsc(p);
    {
        sym_t *q = sym_lookup(p->name, sym_extern);
        if (q) {
            if (!visible && p->sclass == LEX_STATIC) {
                assert(posa[CLS]);
                err_issuep(posa[CLS], ERR_PARSE_INVLINKW, p, " an identifier", &q->pos);
            }
            cmptylist(p, q);
        }
    }
    if (lex_tc != '=') {
        if (p->sclass == LEX_STATIC && !TY_ISFUNC(p->type) && p->type->size == 0)
            err_issuep(&p->pos, ERR_PARSE_INCOMPTYPE, p, " an identifier");
    } else if (TY_ISFUNC(p->type))
        skipinit("function");
    else if (TY_ISVOID(p->type)) {
        p->f.defined = 1;    /* stops diagnostics from doglobal() */
        skipinit("`void' type");
    } else {
        lex_tc = lex_next();
        initglobal(p, lex_ppos, 0);
    }

    return p;
}


/*
 *  installs a local identifier
 */
static sym_t *dcllocal(int sclass, const char *id, ty_t *ty, const lex_pos_t *posa[], int n)
{
    int eqret;
    sym_t *p, *q, *r;

    assert(sclass != LEX_TYPEDEF);
    assert(id);
    assert(ty);
    assert(posa[SPEC]);
    assert(posa[DCLR]);
    assert(pregvar || err_count() > 0);
    assert(pautovar || err_count() > 0);
    assert(ir_cur);
    UNUSED(n);

    if (!sclass)
        sclass = (TY_ISFUNC(ty))? LEX_EXTERN: LEX_AUTO;
    else if (TY_ISFUNC(ty) && sclass != LEX_EXTERN) {
        assert(posa[CLS]);
        err_issuep(posa[CLS], ERR_PARSE_INVCLSID, sclass, ty, id, "");
        sclass = LEX_EXTERN;
    }

    if (sclass != LEX_EXTERN)    /* cannot determine linkage here */
        decl_chkid(id, posa[ID], sym_ident, 0);
    q = sym_lookup(id, sym_ident);
    if (q && !TY_ISUNKNOWN(q->type)) {
        if (SYM_SAMESCP(q, sym_scope)) {
            if (!(q->sclass == LEX_EXTERN && sclass == LEX_EXTERN &&
                (eqret = ty_equiv(q->type, ty, 1)) != 0))
                err_issuep(posa[DCLR], ERR_PARSE_REDECL1, q, " an identifier", &q->pos);
            else if (eqret > 1)
                err_issuep(posa[DCLR], ERR_PARSE_ENUMINT, &q->pos);
        } else if (sclass != LEX_EXTERN || !LINKEDID(q))
            err_issuep(posa[DCLR], ERR_PARSE_HIDEID, q, " an identifier", &q->pos);
    }
    assert(sym_scope >= SYM_SLOCAL);
    p = sym_new(SYM_KORDIN, id, posa[DCLR], sclass, ty, strg_func);
    switch(sclass) {
        case LEX_EXTERN:
            if (!q || !LINKEDID(q))
                q = NULL;    /* q: visible one if any */
            if (!((r = sym_lookup(p->name, sym_global)) != NULL && r->sclass != LEX_ENUM &&
                  r->sclass != LEX_TYPEDEF))
                r = NULL;    /* r: global if any */
            if (q && ((q->scope == SYM_SGLOBAL && q->sclass == LEX_STATIC) ||
                      (q->scope != SYM_SGLOBAL && r && r->sclass == LEX_STATIC))) {
                p->sclass = LEX_STATIC;
                p->scope = SYM_SGLOBAL;
                ir_cur->symgsc(p);
                p->sclass = LEX_EXTERN;
                p->scope = sym_scope;
            } else {
                if (!decl_chkid(p->name, posa[ID], sym_global, 1))
                    decl_chkid(p->name, posa[ID], sym_extern, 1);
                ir_cur->symgsc(p);
            }
            if (q && q->scope != sym_scope) {
                if ((eqret = ty_equiv(q->type, p->type, 1))) {
                    if (eqret > 1)
                        err_issuep(&p->pos, ERR_PARSE_ENUMINT, &q->pos);
                    p->type = ty_compose(q->type, p->type);
                } else
                    err_issuep(&p->pos, ERR_PARSE_REDECL1, p, " an identifier", &q->pos);
            } else if (!q && r) {
                if ((eqret = ty_equiv(p->type, r->type, 1)) == 0)
                    err_issuep(&p->pos, ERR_PARSE_REDECL2, p, " an identifier", &r->pos);
                else if (eqret > 1)
                    err_issuep(&p->pos, ERR_PARSE_ENUMINT, &r->pos);
                if (r->sclass == LEX_STATIC)
                    err_issuep(posa[(posa[CLS])? CLS: SPEC], ERR_PARSE_INVLINKW, p,
                               " an identifier", &r->pos);
            }
            r = sym_lookup(p->name, sym_extern);
            if (r && q != r)
                cmptylist(p, r);
            else {
                r = sym_new(SYM_KEXTERN, p->name, &p->pos, p->sclass, p->type);
                r->tylist = sym_tylist(r->tylist, p);
            }
            break;
        case LEX_STATIC:
            ir_cur->symgsc(p);
            if (lex_tc == '=') {
                lex_tc = lex_next();
                initglobal(p, lex_ppos, 0);
            }
            if (!p->f.defined && p->type->size > 0) {
                decl_defglobal(p, INIT_SEGBSS);
                ir_cur->initspace(p->type->size);
            }
            p->f.defined = 1;
            break;
        case LEX_REGISTER:
            if (pregvar)
                *pregvar = alist_append(*pregvar, p, strg_func);
            regcount++;
            p->f.defined = 1;
            p->f.wregister = 1;
            break;
        case LEX_AUTO:
            if (pautovar)
                *pautovar = alist_append(*pautovar, p, strg_func);
            p->f.defined = 1;
            break;
        default:
            assert(!"invalid storage class -- should never reach here");
            break;
    }
    if (lex_tc == '=') {
        tree_t *e;
        err_entersite(lex_cpos);    /* enters with = */
        if (TY_ISFUNC(p->type) || sclass == LEX_EXTERN)
            skipinit((TY_ISFUNC(p->type))? "function": "local extern");
        else {
            lex_tc = lex_next();
            if (!TY_ISUNKNOWN(p->type)) {
                stmt_defpoint(NULL);
                if (TY_ISSCALAR(p->type) || (TY_ISSTRUNI(p->type) && lex_tc != '{')) {
                    if (lex_tc == '{') {
                        lex_tc = lex_next();
                        e = expr_asgn(0, 0, 1);
                        if (lex_tc == ',')
                            lex_tc = lex_next();
                        lex_extracomma(',', "initializer", 1);
                        err_expect('}');
                    } else
                        e = expr_asgn(0, 0, 1);
                } else {
                    sym_t *t1;
                    t1 = sym_new(SYM_KGEN, LEX_STATIC, p->type, SYM_SGLOBAL);
                    initglobal(t1, err_getppos(), 1);
                    if (TY_ISARRAY(p->type) && p->type->size == 0 && t1->type->size > 0) {
                        err_entersite(NULL);    /* enters with turning off */
                        p->type = ty_array_s(p->type->type, t1->type->size/t1->type->type->size);
                        err_exitsite();    /* exits from turning off */
                    }
                    e = tree_id_s(t1);
                }
                assert(!TY_ISFUNC(p->type));
                dag_walk(tree_asgnid_s(p, e), 0, 0);
                if (p->type->size > 0)
                    p->f.set = 1;
            } else
                init_skip();
        }
        err_exitsite();    /* exits from = */
    }
    if (!TY_ISFUNC(p->type) && p->f.defined && p->type->size == 0)
        err_issuep(&p->pos, ERR_PARSE_INCOMPTYPE, p, " an identifier");

    return p;
}


/*
 *  applies various checks to symbols;
 *  called at each param scope(P), each local scope(L) and the file scope(F);
 *  ASSUMPTION: only scalar types can be stored in registers;
 *  TODO: warn of unreferenced objects after being assigned
 */
static void checkref(sym_t *p, void *cl)
{
    sym_t *q;

    assert(p);
    UNUSED(cl);

    if (TY_ISVOID(p->type) && p->sclass == LEX_EXTERN && p->ref > 0 &&
        ((q=sym_lookup(p->name, sym_global)) == NULL || p == q) && !err_experr())    /* L, F */
        err_issuep(&p->pos, ERR_PARSE_VOIDOBJ, p, "");
    if (p->scope >= SYM_SPARAM && TY_ISVOLATILE(p->type))    /* P, L */
        p->f.addressed = 1;
    if (!GENNAME(p->name) && p->f.defined && p->ref == 0 && !TY_ISVOID(p->type) &&
        !err_experr()) {    /* P, L, F */
        if (p->sclass == LEX_STATIC)
            err_issuep(&p->pos, ERR_PARSE_REFSTATIC, p, " identifier");
        else if (p->scope == SYM_SPARAM)
            err_issuep(&p->pos, ERR_PARSE_REFPARAM, p, "");
        else if (p->scope >= SYM_SLOCAL && p->sclass != LEX_EXTERN)
            err_issuep(&p->pos, ERR_PARSE_REFLOCAL, p, " identifier");
        p->f.reference = 1;
    }
    if (p->f.set == 1 && !p->f.reference && !err_experr()) {    /* P, L, F */
        if (p->sclass == LEX_STATIC)
            err_issuep(&p->pos, ERR_PARSE_SETNOREFS, p, " identifier");
        else if (p->scope == SYM_SPARAM)
            err_issuep(&p->pos, ERR_PARSE_SETNOREFP, p, "");
        else if (p->scope >= SYM_SLOCAL && p->sclass != LEX_EXTERN)
            err_issuep(&p->pos, ERR_PARSE_SETNOREFL, p, " identifier");
    }
    if (p->sclass == LEX_AUTO &&
        ((p->scope == SYM_SPARAM && regcount == 0) || p->scope >= SYM_SLOCAL) &&
        !p->f.addressed && TY_ISSCALAR(p->type) && p->ref >= 3.0)    /* P, L */
        p->sclass = LEX_REGISTER;
    if (p->scope >= SYM_SLOCAL && p->sclass == LEX_EXTERN) {    /* L */
        q = sym_lookup(p->name, sym_extern);
        assert(q);
        sym_ref(q, p->ref);
    }
    if (sym_scope == SYM_SGLOBAL && p->sclass == LEX_STATIC && !p->f.defined &&
        TY_ISFUNC(p->type) && p->ref > 0)    /* F */
        err_issuep(&p->pos, ERR_PARSE_UNDSTATIC, p, " identifier");
    assert(!(sym_scope == SYM_SGLOBAL && p->sclass == LEX_STATIC && !p->f.defined &&
             !TY_ISFUNC(p->type)));
}


/*
 *  handles symbols in sym_extern at the end of compilation
 */
static void doextern(sym_t *p, void *cl)
{
    sym_t *q;

    assert(p);
    assert(ir_cur);
    UNUSED(cl);

    if ((q = sym_lookup(p->name, sym_ident)) != NULL)
        sym_ref(q, p->ref);
    else {
        ir_cur->symgsc(p);
        ir_cur->import(p);
    }
}


/*
 *  handles symbols in sym_global at the end of compilation
 */
static void doglobal(sym_t *p, void *cl)
{
    assert(p);
    assert(ir_cur);
    UNUSED(cl);

    assert(err_count() > 0 || !TY_ISFUNC(p->type) || p->sclass != LEX_AUTO);
    if (!p->f.defined && p->sclass == LEX_EXTERN)
        ir_cur->import(p);
    else if (!p->f.defined && !TY_ISFUNC(p->type) &&
             (p->sclass == LEX_AUTO || p->sclass == LEX_STATIC)) {
        if (TY_ISARRAY(p->type) && p->type->size == 0 && p->type->type->size > 0) {
            err_entersite(&p->pos);    /* enters with p->pos */
            p->type = ty_array_s(p->type->type, 1);
            err_exitsite();    /* exits from p->pos */
        }
        if (p->type->size > 0) {
            decl_defglobal(p, INIT_SEGBSS);
            ir_cur->initspace(p->type->size);
        } else if (p->sclass != LEX_STATIC)    /* static incomplete checked in dclglobal() */
            err_issuep(&p->pos, ERR_PARSE_INCOMPTYPE, p, " an identifier");
        p->f.defined = 1;
    }
    if (main_opt()->proto && !TY_ISFUNC(p->type) && !GENSYM(p) && p->f.defined) {
        int anonym;
        fprintf(stderr, "%s;\n", ty_outdecl(p->type, p->name, &anonym, 0));
        if (anonym)
            err_issuep(&p->pos, ERR_TYPE_ERRPROTO, p, " a variable");
    }
}


/*
 *  handles constants at the end of compilation
 */
static void doconst(sym_t *p, void *cl)
{
    assert(p);
    assert(ir_cur);
    UNUSED(cl);

    if (p->u.c.loc) {
        assert(p->u.c.loc->u.seg == 0);
        assert(p->u.c.loc->type->size > 0);
        decl_defglobal(p->u.c.loc, INIT_SEGLIT);
        if (TY_ISARRAY(p->type))
            ir_cur->initstr(p->type->size, p->u.c.v.hp);
        else
            ir_cur->initconst(op_sfx(p->type), p->u.c.v);
        p->u.c.loc->f.defined = 1;
    }
}


/*
 *  finalizes compilation by applying various checks to symbols
 */
void (decl_finalize)(void)
{
    sym_foreach(sym_extern, SYM_SGLOBAL, doextern, NULL);
    sym_foreach(sym_ident,  SYM_SGLOBAL, doglobal, NULL);
    sym_foreach(sym_ident,  SYM_SGLOBAL, checkref, NULL);
    sym_foreach(sym_const,  SYM_SCONST,  doconst,  NULL);
}


/*
 *  checks if all used labels defined
 */
static void checklab(sym_t *p, void *cl)
{
    assert(p);
    UNUSED(cl);

    if (!p->f.wregister) {
        assert(p->f.defined);
        err_issuep(&p->pos, ERR_STMT_UNUSEDLAB, p, "");
    } else if (!p->f.defined)
        err_issuep(&p->pos, ERR_STMT_UNDEFLAB, p, "");
}


/*
 *  parses a compound statement including function bodies
 */
void (decl_compound)(int loop, stmt_swtch_t *swp, int lev)
{
    ty_t *rty;
    stmt_t *cp;
    long nreg;
    unsigned stmtseen;
    lex_pos_t posstmt;    /* statement */
    alist_t *autovar, *regvar;

    assert(ir_cur);

    dag_walk(NULL, 0, 0);
    cp = stmt_new(STMT_BLOCKBEG);
    sym_enterscope();
    assert(sym_scope >= SYM_SLOCAL);
    autovar = regvar = NULL;
    rty = ty_freturn(decl_cfunc->type);

    if (sym_scope == SYM_SLOCAL && ir_cur->f.want_callb && TY_ISSTRUNI(rty)) {
        decl_retv = sym_new(SYM_KGEN, LEX_AUTO, ty_ptr(TY_UNQUAL(rty)), sym_scope);
        decl_retv->f.defined = 1;
        sym_ref(decl_retv, 1);
        regvar = alist_append(regvar, decl_retv, strg_func);
    }
    err_expect('{');
    posstmt = *lex_cpos;
    stmtseen = 0;
    do {
        if (lex_ispdecl()) {
            if (main_opt()->std == 1 && stmtseen == 1)
                err_issuep(lex_cpos, ERR_PARSE_MIXDCLSTMT);
            stmt_chkreach();
            pautovar = &autovar;
            pregvar = &regvar;
            do
                decl(dcllocal);
            while(lex_ispdecl());
        }
        if (lex_ispstmt()) {
            stmtseen++;
            do
                stmt_stmt(loop, swp, lev, &posstmt, NULL, 0);
            while(lex_ispstmt());
        }
        if (!lex_issdecl() && lex_tc != '}' && lex_tc != LEX_EOI) {
            err_issuep(lex_cpos, ERR_PARSE_INVDCLSTMT);
            err_skipto('}', err_sset_declb);
        }
    } while(lex_issdecl() || lex_issstmt());
    {
        long i;
        node_t *a = alist_toarray(autovar, strg_func);    /* sym_t */
        nreg = alist_length(regvar);
        for (i = 0; a[i]; i++)
            regvar = alist_append(regvar, a[i], strg_func);
        cp->u.block.local = alist_toarray(regvar, strg_func);
    }
    dag_walk(NULL, 0, 0);
    sym_foreach(sym_ident, sym_scope, checkref, NULL);
    {
        long i = nreg, j;
        sym_t *p;
        while ((p = cp->u.block.local[i]) != NULL) {
            for (j = i; j > nreg && S(cp->u.block.local[j-1])->ref < p->ref; j--)
                cp->u.block.local[j] = cp->u.block.local[j-1];
            cp->u.block.local[j] = p;
            i++;
        }
    }
    cp->u.block.scope = sym_scope;
    cp->u.block.ident = sym_ident;
    cp->u.block.type = sym_type;
    stmt_new(STMT_BLOCKEND)->u.begin = cp;
    sym_exitscope();
}


/*
 *  exits a parameter scope
 */
static void exitparam(node_t param[])    /* sym_t */
{
    assert(param);

    if (param[0] && !S(param[0])->f.defined)
        err_issuep(&S(param[0])->pos, ERR_PARSE_EXTRAPARAM);
    if (sym_scope > SYM_SPARAM)
        sym_exitscope();
    sym_exitscope();
}


/*
 *  provides a callback for sym_foreach() in funcdefn()
 */
static void oldparam(sym_t *p, void *cl)
{
    int i;
    node_t *callee = cl;    /* sym_t */

    assert(p);
    assert(cl);
    assert(p->name);

    if (p->sclass == LEX_ENUM)
        return;
    for (i = 0; callee[i]; i++) {
        /* cannot assert(p->sclass != LEX_TYPEDEF) due to unknown type (#59) */
        if (p->name == S(callee[i])->name) {
            callee[i] = p;
            return;
        }
    }
    if (*p->name != '#' && !TY_ISUNKNOWN(p->type))
        err_issuep(&p->pos, ERR_PARSE_NOPARAM, p, "");
}


/*
 *  checks the definition of main()
 */
static void chkmain(void)
{
    int n;
    ty_t *ty, *rty;

    assert(decl_mainfunc);
    assert(decl_cfunc);
    assert(decl_callee);
    assert(ty_inttype);    /* ensures types initialized */

    ty = decl_cfunc->type;
    rty = ty_freturn(ty);
    for (n = 0; decl_callee[n]; n++)
        continue;
    if (!(rty->t.type == ty_inttype &&
          (n == 0 ||
           (n == 2 && S(decl_callee[0])->type->t.type == ty_inttype &&
                      TY_ISPTR(S(decl_callee[1])->type) &&
                      TY_ISPTR(S(decl_callee[1])->type->type) &&
                      S(decl_callee[1])->type->type->type->t.type == ty_chartype &&
                      !ty_variadic(ty)))))
        err_issuep(&decl_cfunc->pos, ERR_XTRA_INVMAIN, ty, decl_cfunc->name, &n);
}


/*
 *  parses a function definition
 */
static void funcdefn(int sclass, const char *id, ty_t *ty, node_t param[],    /* sym_t */
                     const lex_pos_t *posa[])
{
    int i, n;
    int eqret;
    ty_t *rty;
    sym_t *p;
    node_t *callee, *caller;    /* sym_t */

    assert(id);
    assert(ty);
    assert(param);
    assert(posa[SPEC]);
    assert(posa[DCLR]);
    assert(TY_ISFUNC(ty));
    assert(ty_inttype);    /* ensures types initialized */
    assert(ir_cur);

    in_enterfunc();
    rty = ty_freturn(ty);
    assert(!TY_ISFUNC(rty) && !TY_ISARRAY(rty));
    if (TY_ISSTRUNI(rty) && rty->size == 0)
        err_issuep(posa[SPEC], ERR_PARSE_INCOMPRET);
    if (TY_ISQUAL(rty))
        err_issuep(posa[SPEC], ERR_PARSE_QUALFRET);
    for (n = 0; param[n]; n++)
        continue;
    if (n > 0 && !S(param[n-1])->name)
        param[--n] = NULL;
    if (n > TL_PARAM_STD) {
        err_issuep(&S(param[TL_PARAM_STD])->pos, ERR_PARSE_MANYPARAM);
        err_issuep(&S(param[TL_PARAM_STD])->pos, ERR_PARSE_MANYPSTD, (long)TL_PARAM_STD);
    }
    if (ty->u.f.oldstyle) {
        lex_pos_t pos;
        node_t *proto = NULL;    /* ty_t */

        p = sym_lookup(id, sym_global);
        if (p && TY_ISFUNC(p->type) && p->f.defined && ty_equiv(ty, p->type, 1))
            err_issuep(posa[DCLR], ERR_PARSE_REDEF, p, " an identifier", &p->pos);
        if (p && TY_ISFUNC(p->type) && p->type->u.f.proto) {
            pos = p->pos;
            proto = p->type->u.f.proto;
        }
        decl_cfunc = dclglobal(sclass, id, ty, posa, 0);    /* may overwrite p's fields */

        inparam++;
        caller = param;
        callee = ARENA_ALLOC(strg_func, (n+1)*sizeof(*callee));
        memcpy(callee, caller, (n+1)*sizeof(*callee));
        assert(sym_scope == SYM_SPARAM);
        while (lex_tc != '{' && lex_tc != LEX_EOI && lex_issdecl()) {
            if (lex_issdecl())
                decl(dclparam);
            else {
                err_issuep(lex_cpos, ERR_PARSE_INVDECL);
                err_skipto('{', err_sset_initb);
                if (lex_tc == ';')    /* avoids infinite loop */
                    lex_tc = lex_next();
            }
        }
        sym_foreach(sym_ident, SYM_SPARAM, oldparam, callee);
        for (i = 0; (p = callee[i]) != NULL; i++) {
            if (!p->f.defined) {
                const lex_pos_t *posa[LEN];
                SETPOSA(&p->pos, NULL, &p->pos, NULL);
                callee[i] = dclparam(0, p->name, ty_inttype, posa, 0);
            }
            *S(caller[i]) = *p;
            S(caller[i])->sclass = LEX_AUTO;
            S(caller[i])->type = ty_apromote(p->type);
        }
        if (proto) {
            for (i = 0; caller[i] && proto[i]; i++) {
                eqret = ty_equiv(TY_UNQUAL(T(proto[i])), TY_UNQUAL(S(caller[i])->type), 1);
                if (eqret == 0)
                    break;
                else if (eqret > 1)
                    err_issuep(&S(caller[i])->pos, ERR_PARSE_ENUMINT, &pos);
            }
            if (caller[i])
                err_issuep(&S(caller[i])->pos, ERR_PARSE_PARAMMATCH, &pos);
            else if (proto[i])
                err_issuep(lex_cpos, ERR_PARSE_PARAMMATCH, &pos);
        } else {
            proto = ARENA_ALLOC(strg_perm, (n+1)*sizeof(*proto));
            err_issuep(posa[DCLR], ERR_PARSE_NOPROTO);
            for (i = 0; i < n; i++)
                proto[i] = S(caller[i])->type;
            proto[i] = NULL;
            err_entersite(posa[DCLR]);    /* enters with declarator */
            decl_cfunc->type = ty_func_s(rty, proto, 1);
            decl_cfunc->type->u.f.implint = ty->u.f.implint;
            err_exitsite();    /* exits from declarator */
        }
        inparam--;
        assert(inparam >= 0);
    } else {
        callee = param;
        caller = ARENA_ALLOC(strg_func, (n+1)*sizeof(*caller));
        for (i = 0; (p = callee[i]) != NULL && p->name; i++) {
            if (p->name == id && !sym_lookup(id, sym_global))
                err_issuep(&p->pos, ERR_PARSE_HIDEID, p, " an identifier", posa[DCLR]);
            caller[i] = ARENA_ALLOC(strg_func, sizeof(*S(caller[i])));
            *S(caller[i]) = *p;
            S(caller[i])->type = ty_ipromote(p->type);
            S(caller[i])->sclass = LEX_AUTO;
            if (GENNAME(p->name))
                err_issuep(&p->pos, ERR_PARSE_NOPARAMID, i+1);
        }
        caller[i] = NULL;
        p = sym_lookup(id, sym_ident);
        if (p && TY_ISFUNC(p->type) && p->f.defined && ty_equiv(ty, p->type, 1))
            err_issuep(posa[DCLR], ERR_PARSE_REDEF, p, " an identifier", &p->pos);
        decl_cfunc = dclglobal(sclass, id, ty, posa, 0);
    }

    for (i = 0; (p = callee[i]) != NULL; i++)
        if (p->type->size == 0) {
            err_issuep(&p->pos, ERR_PARSE_INCOMPARAM, p, "");
            S(caller[i])->type = p->type = ty_inttype;
        }
    decl_cfunc->u.f.label = sym_genlab(1);
    decl_cfunc->u.f.pt = *lex_cpos;
    decl_cfunc->f.defined = 1;
    decl_callee = callee;
    if (sclass != LEX_STATIC && strcmp(id, "main") == 0) {
        decl_mainfunc = 1;
        chkmain();
    }
    if (main_opt()->xref)
        sym_use(decl_cfunc, &decl_cfunc->pos);
    if (main_opt()->proto) {
        int anonym;
        fprintf(stderr, "%s;\n", ty_outdecl(decl_cfunc->type, id, &anonym, 0));
        if (anonym)
            err_issuep(&decl_cfunc->pos, ERR_TYPE_ERRPROTO, err_idsym(id), " a function");
    }
    sym_label = sym_table(NULL, SYM_SLABEL);
    stmt_lab = sym_table(NULL, SYM_SLABEL);
    expr_refinc = 1.0;
    regcount = 0;
    stmt_list = &stmt_head;
    stmt_list->next = NULL;
    stmt_defpoint(NULL);
    if (!ir_cur->f.want_callb && TY_ISSTRUNI(rty))
        decl_retv = sym_new(SYM_KGEN, LEX_AUTO, ty_ptr(TY_UNQUAL(rty)), SYM_SPARAM);
    decl_compound(0, NULL, 0);
    {
        stmt_t *cp;
        ty_t *uty = TY_UNQUAL(rty)->t.type;

        for (cp = stmt_list; cp->kind < STMT_LABEL; cp = cp->prev)
            continue;
        if (cp->kind != STMT_JUMP) {
            if (DECL_NORET(ty))
                err_issuep(lex_cpos, ERR_STMT_NORETURN);
            stmt_retcode(NULL, lex_cpos);
        }
    }
    stmt_deflabel(decl_cfunc->u.f.label);
    stmt_defpoint(NULL);
    assert(sym_scope == SYM_SPARAM);
    sym_foreach(sym_ident, sym_scope, checkref, NULL);
    if (!ir_cur->f.want_callb && TY_ISSTRUNI(rty)) {
        node_t *a;    /* sym_t */
        a = ARENA_ALLOC(strg_func, (n+2) * sizeof(*a));
        a[0] = decl_retv;
        memcpy(&a[1], callee, (n+1) * sizeof(*callee));
        callee = a;
        a = ARENA_ALLOC(strg_func, (n+2) * sizeof(*a));
        a[0] = ARENA_ALLOC(strg_func, sizeof(*S(a[0])));
        *S(a[0]) = *decl_retv;
        memcpy(&a[1], caller, (n+1) * sizeof(*callee));
        caller = a;
    }
    if (!ir_cur->f.want_argb)
        for (i = 0; caller[i]; i++)
            if (TY_ISSTRUNI(S(caller[i])->type)) {
                S(caller[i])->type = ty_ptr(S(caller[i])->type);
                S(callee[i])->type = ty_ptr(S(callee[i])->type);
                S(caller[i])->f.structarg = S(callee[i])->f.structarg = 1;
            }
    if (main_opt()->glevel)
        for (i = 0; callee[i]; i++)
            S(callee[i])->sclass = LEX_AUTO;
    if (decl_cfunc->sclass != LEX_STATIC)
        ir_cur->export(decl_cfunc);
    init_swtoseg(INIT_SEGCODE);
    ir_cur->function(decl_cfunc, caller, callee, decl_cfunc->u.f.ncall);
    sym_foreach(stmt_lab, SYM_SLABEL, checklab, NULL);
    sym_exitscope();
    err_expect('}');
    in_exitfunc();
    sym_label = stmt_lab = NULL;
    clear_declobj();
    err_cleareff();
}


/*
 *  parses declarations
 */
static void decl(sym_t *(*dcl)(int, const char *, ty_t *, const lex_pos_t *[], int))
{
    int n = 0;
    int impl, sclass;
    ty_t *ty, *ty1;
    lex_pos_t posspec,    /* declaration specifier */
              poscls,     /* storage-class specifier */
              posdclr,    /* declarator */
              posid;      /* identifier */
    const lex_pos_t *posa[LEN];

    assert(dcl);

    strunilev = 0;
    SETPOSA(&posspec, NULL, &posdclr, &posid);
    posspec = *lex_cpos;
    ty = specifier(&sclass, &poscls, &impl);
    if (sclass)
        posa[CLS] = &poscls;
    if (lex_isadcl()) {
        const char *id = NULL;
        posdclr = *lex_cpos;
        if (sym_scope == SYM_SGLOBAL) {
            node_t *param = NULL;    /* sym_t */
            ty1 = dclr(ty, &id, &param, 0, &posdclr, &posid);
            if (param && id && TY_ISFUNC(ty1) &&
                (lex_tc == '{' ||
                 (param[0] && !S(param[0])->f.defined && lex_issdecl()))) {
                if (sclass == LEX_TYPEDEF) {
                    err_issuep(&poscls, ERR_PARSE_TYPEDEFF);
                    sclass = LEX_EXTERN;
                }
                if (ty1->u.f.oldstyle) {
                    sym_exitscope();
                    sym_enterscope();
                }
                ty1->u.f.implint = (impl >> 1);
                funcdefn(sclass, id, ty1, param, posa);
                return;
            } else if (param)
                exitparam(param);
        } else
            ty1 = dclr(ty, &id, NULL, 0, &posdclr, &posid);
        while (1) {
            if (posdclr.g.f) {
                if (!ty_hasproto(ty1))
                    err_issuep(&posdclr, ERR_PARSE_NOPROTO);
                if (!id) {
                    err_issuep(&posdclr, ERR_PARSE_NOID);
                    skipinit(NULL);
                } else if (sclass == LEX_TYPEDEF && dcl != dclparam)
                    typedefsym(id, ty1, posa[DCLR], impl & 1);
                else
                    dcl(sclass, id, ty1, posa, n++);
            }
            if (lex_tc != ',')
                break;
            lex_tc = lex_next();
            if (lex_extracomma(';', "declarator", 0))
                break;
            id = NULL;
            if (lex_isadcl()) {
                posdclr = *lex_cpos;
                ty1 = dclr(ty, &id, NULL, 0, &posdclr, &posid);
            } else {
                posdclr.g.f = NULL;
                err_issuep(lex_epos(), ERR_PARSE_NODCLR, "");
                skipinit(NULL);
            }
        }
    } else {
        if (sclass)
            err_issuep(&poscls, ERR_PARSE_NOUSECLS, sclass);
        if (!TY_ISENUM(ty) && (!TY_ISSTRUNI(ty) || GENNAME(TY_UNQUAL(ty)->u.sym->name)))
            err_issuep(lex_epos(), ERR_PARSE_EMPTYDECL);
        else if (inparam)
            err_issuep(lex_epos(), ERR_PARSE_DECLPARAM);
        else if (lex_tc == '=')
            err_issuep(lex_cpos, ERR_PARSE_UNUSEDINIT);
        skipinit(NULL);
    }
    err_test(';', (sym_scope >= SYM_SPARAM)? err_sset_declb: err_sset_declf);
}


/*
 *  parses a translation unit
 */
void (decl_program)(void)
{
    sym_scope = SYM_SGLOBAL;

    if (lex_tc != LEX_EOI) {
        do {
            if (lex_issdecl() || lex_isdcl()) {
                if (lex_isdcl() && !lex_istype())
                    err_issuep(lex_cpos, ERR_PARSE_NODECLSPEC);
                decl(dclglobal);
                ARENA_FREE(strg_stmt);
                if (!main_opt()->glevel && !main_opt()->xref)
                    ARENA_FREE(strg_func);
            } else if (lex_tc == ';') {
                err_issuep(lex_epos(), ERR_PARSE_EMPTYDECL);
                lex_tc = lex_next();
            } else {
                err_issuep(lex_cpos, ERR_PARSE_INVDECL);
                err_skipto(0, err_sset_decl);
                if (lex_tc == ';')    /* avoids "empty declaration" warning */
                    lex_tc = lex_next();
            }
        } while(lex_tc != LEX_EOI);
    } else
        err_issuep(lex_cpos, ERR_INPUT_EMPTYFILE);
}


/*
 *  parses a type name
 */
ty_t *(decl_typename)(void)
{
    ty_t *ty;
    lex_pos_t posdclr;    /* declarator */
    int dummy;

    ty = specifier(NULL, NULL, &dummy);
    posdclr = *lex_cpos;
    if (lex_tc == '*' || lex_tc == '(' || lex_tc == '[') {
        ty = dclr(ty, NULL, NULL, 1, &posdclr, NULL);
        if (!ty_hasproto(ty))
            err_issuep(&posdclr, ERR_PARSE_NOPROTO);
    }

    return ty;
}


/*
 *  consumes an erroneous local declaration
 */
void (decl_errdecl)(void)
{
    assert(err_count() > 0);
    decl(dcllocal);
}

/* end of decl.c */
