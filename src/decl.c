/*
 *  declaration parsing
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* fprintf, stderr */
#include <string.h>        /* memcpy */
#include <cbl/arena.h>     /* ARENA_ALLOC, ARENA_CALLOC, ARENA_FREE */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_int */

#include "alist.h"
#include "clx.h"
#include "common.h"
#include "dag.h"
#include "err.h"
#include "expr.h"
#include "init.h"
#include "ir.h"
#include "lex.h"
#include "main.h"
#include "op.h"
#include "sset.h"
#include "simp.h"
#include "stmt.h"
#include "strg.h"
#include "sym.h"
#include "ty.h"
#include "decl.h"

/* field calculations;
   ASSUMPTION: bit-field is signed or unsigned int */
#define ADD(x, n)         (((x) > SSZ_MAX-(n))? (ovf=1, (x)): (x)+(n))
#define CHKOVERFLOW(x, n) ((void)ADD(x, n))
#define BITUNITS(n)       (((n)+TG_CHAR_BIT-1) / TG_CHAR_BIT)
#define BITUNITD(n)       (((n) + TG_CHAR_BIT*ty_unsignedtype->size - 1) /    \
                           (TG_CHAR_BIT*ty_unsignedtype->size) * ty_unsignedtype->size)
#define BIT2BYTE(n)       ((ir_cur->f.little_bit == ir_cur->f.little_endian)?    \
                              BITUNITS(n): BITUNITD(n))

/* maps size, sign, type to array index */
#ifdef SUPPORT_LL
#define SIZEIDX(x) (((x) == LEX_SHORT)? 1: ((x) == LEX_LONG)? 2: ((x) == LEX_LLONG)? 3: 0)
#else    /* !SUPPORT_LL */
#define SIZEIDX(x) (((x) == LEX_SHORT)? 1: ((x) == LEX_LONG)? 2: 0)
#endif    /* SUPPORT_LL */
#define SIGNIDX(x) (((x) == LEX_SIGNED)? 1: ((x) == LEX_UNSIGNED)? 2: 0)
#define TYPEIDX(x) (((x) == LEX_CHAR)? 1: ((x) == LEX_INT)? 2: ((x) == LEX_FLOAT)? 3: \
                    ((x) == LEX_DOUBLE)? 4: 0)

/* resets objects related to decl_cfunc */
#define clear_declobj() (decl_retv=decl_cfunc=NULL, decl_callee=NULL, decl_mainfunc=0)

/* checks if symbol denotes identifier with linkage */
#define LINKEDID(p) ((p)->sclass != LEX_ENUM && (p)->sclass != LEX_TYPEDEF &&     \
                     ((p)->scope == SYM_SGLOBAL || (p)->sclass == LEX_EXTERN))

#define S(p) ((sym_t *)(p))    /* shorthand for cast to sym_t * */
#define T(p) ((ty_t *)(p))     /* shorthand for cast to ty_t * */


enum { PS, PC, PD, PI, LEN };    /* used with const lmap_t * array;
                                    specifier, storage class, declarator, id */
typedef void *node_t;            /* refers to void * for readability */


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
static ty_t *dclr(ty_t *, const char **, node_t **,    /* sym_t */
                  int, const lmap_t *[]);
static void exitparam(node_t []);    /* sym_t */
static void decl(sym_t *(*)(int, const char *, ty_t *, const lmap_t *[], int), const char *);
static sym_t *typedefsym(const char *, ty_t *, const lmap_t *, int);


#define INIT(x) &(ty_ ## x ## type)

/*
 *  parses a declaration specifier
 */
static ty_t *specifier(int *sclass, const lmap_t *posa[], int *impl, const char *tyla)
{
    ty_t *ty = NULL;
    enum { SN = -1, SC, SCN, SVL, SSG, SSZ, STY, SLNG, SLEN } n;
    int s[SLEN] = { 0, };
    const lmap_t *posst, *posn[SLEN] = { NULL, };

    assert(posa);
    assert(impl);

    *impl = 1;
    posst = clx_cpos;
    posa[PS] = posa[PC] = NULL;
    while (1) {
        int tt = clx_tc;
        const lmap_t *pos = clx_cpos;
        switch(clx_tc) {
            /* storage-class specifier */
            case LEX_AUTO:
            case LEX_REGISTER:
            case LEX_STATIC:
            case LEX_EXTERN:
            case LEX_TYPEDEF:
                if (!s[SC] && (s[SCN] || s[SVL] || s[SSG] || s[SSZ] || s[STY]))
                    err_dpos(clx_cpos, ERR_PARSE_CLSFIRST, clx_tc);
                if (!sclass) {
                    err_dpos(clx_cpos, ERR_PARSE_CLS);
                    s[SC] = 0;    /* shuts up ERR_PARSE_INVUSE */
                } else if (!posa[PC])
                    posa[PC] = clx_cpos;
                n = SC;
                clx_tc = clx_next();
                break;
            /* type qualifier */
            case LEX_CONST:
                n = SCN;
                clx_tc = clx_next();
                break;
            case LEX_VOLATILE:
                n = SVL;
                clx_tc = clx_next();
                break;
            /* type specifier */
            case LEX_SIGNED:
            case LEX_UNSIGNED:
                *impl = 0;
                n = SSG;
                clx_tc = clx_next();
                break;
            case LEX_LONG:
            case LEX_SHORT:
                n = SSZ;
                clx_tc = clx_next();
                break;
            case LEX_VOID:
            case LEX_CHAR:
            case LEX_INT:
            case LEX_FLOAT:
            case LEX_DOUBLE:
                n = STY;
                clx_tc = clx_next();
                break;
            case LEX_ENUM:
                if (s[STY])
                    n = SN;
                else {
                    n = STY;
                    ty = enumdcl();
                }
                break;
            case LEX_STRUCT:
            case LEX_UNION:
                if (s[STY])
                    n = SN;
                else {
                    n = STY;
                    ty = structdcl(clx_tc);
                }
                break;
            case LEX_ID:
                if (clx_istype(tyla) && !s[STY] && !s[SSG] && !s[SSZ]) {
                    if (!clx_sym) {
                        err_dpos(clx_cpos, ERR_PARSE_UNKNOWNTY, clx_tok);
                        clx_sym = typedefsym(clx_tok, ty_unknowntype, clx_cpos, 0);
                    }
                    if (main_opt()->xref)
                        sym_use(clx_sym, clx_cpos);
                    ty = clx_sym->type;
                    *impl = ty->t.plain;
                    n = STY;
                    clx_tc = clx_next();
                } else
                    n = SN;
                break;
            default:
                n = SN;
                break;
        }
        if (n == SN) {
            if (posa[PS])
                posa[PS] = lmap_range(posa[PS], clx_ppos);
            break;
        }
        posa[PS] = posst;
        if (!posn[n])
            posn[n] = pos;
        if (s[n]) {
#ifdef SUPPORT_LL
            if (n == SSZ && s[n] == LEX_LONG && tt == LEX_LONG) {
                s[n] = LEX_LLONG;
                posn[SLNG] = pos;
            } else
#endif    /* SUPPORT_LL */
            if (n == SCN || n == SVL)
                err_dmpos(clx_ppos, ERR_TYPE_DUPQUAL, posn[n], NULL, tt);
            else
#ifdef SUPPORT_LL
            if (n == SSZ && s[SSZ] == LEX_LLONG)
                err_dmpos(clx_ppos, ERR_PARSE_INVUSE, posn[SSZ], posn[SLNG], NULL, tt);
            else
#endif    /* SUPPORT_LL */
                err_dmpos(clx_ppos, ERR_PARSE_INVUSE, posn[n], NULL, tt);
        } else
            s[n] = tt;
    }
    if (sclass)
        *sclass = s[SC];
    {
#ifdef SUPPORT_LL
        static ty_t **tab[4][3][5] = {    /* none/short/l/ll * none/signed/unsigned * v/c/i/f/d */
#else    /* !SUPPORT_LL */
        static ty_t **tab[3][3][5] = {    /* none/short/long * none/signed/unsigned * v/c/i/f/d */
#endif    /* SUPPORT_LL */
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
#ifdef SUPPORT_LL
            },
            {    /* llong */
 /* none */     { (INIT(void)), (INIT(char)),  INIT(llong),    (INIT(double)), (INIT(ldouble)) },
 /* signed */   { (INIT(void)), (INIT(schar)), INIT(llong),    (INIT(double)), (INIT(ldouble)) },
 /* unsigned */ { (INIT(void)), (INIT(uchar)), INIT(ullong),   (INIT(double)), (INIT(ldouble)) }
#endif    /* SUPPORT_LL */
            }
        };

        if (!s[STY]) {
            if (!s[SSZ] && !s[SSG]) {
                const lmap_t *pos = lmap_pin(clx_cpos);
                (void)(err_dpos(pos, ERR_PARSE_DEFINT) &&
                       err_dpos(pos, ERR_PARSE_DEFINTSTD));
                *impl |= (1 << 1);
            }
            s[STY] = LEX_INT;
        }
#ifdef SUPPORT_LL
        if (((s[SSZ] == LEX_SHORT || s[SSZ] == LEX_LLONG) && s[STY] != LEX_INT) ||
#else    /* !SUPPORT_LL */
        if ((s[SSZ] == LEX_SHORT && s[STY] != LEX_INT) ||
#endif    /* SUPPORT_LL */
            (s[SSZ] == LEX_LONG && s[STY] != LEX_INT && s[STY] != LEX_DOUBLE)) {
#ifdef SUPPORT_LL
            if (s[SSZ] == LEX_LLONG)
                err_dmpos(posn[SSZ], ERR_PARSE_INVTYPE, posn[SLNG], posn[STY], NULL,
                          s[SSZ], s[STY]);
            else
#endif    /* SUPPORT_LL */
                err_dmpos(posn[SSZ], ERR_PARSE_INVTYPE, posn[STY], NULL, s[SSZ], s[STY]);
        } else if (s[SSG] && s[STY] != LEX_INT && s[STY] != LEX_CHAR)
            err_dmpos(posn[SSG], ERR_PARSE_INVTYPE, posn[STY], NULL, s[SSG], s[STY]);
        if (!ty) {
            ty = *tab[SIZEIDX(s[SSZ])][SIGNIDX(s[SSG])][TYPEIDX(s[STY])];
            assert(ty);
        }
#ifdef SUPPORT_LL
        if (main_opt()->std == 1 && (TY_ISLLONG(ty) || TY_ISULLONG(ty)) && !ty->t.name)
            err_dmpos(posn[SSZ], ERR_PARSE_LLONGINC90, posn[SLNG], NULL);
#endif    /* SUPPORT_LL */
        if (s[SCN])
            ty = ty_qual(TY_CONST, ty, 0, posn[SCN]);
        if (s[SVL])
            ty = ty_qual(TY_VOLATILE, ty, 0, posn[SVL]);
    }

    return ty;
}

#undef INIT


/*
 *  checks the length of a declared identifier
 */
int (decl_chkid)(const char *id, const lmap_t *pos, sym_tab_t *tp, int glb)
{
    sym_t *p;

    assert(id);
    assert(tp);

    if ((p = sym_clookup(id, tp, glb)) != NULL && (!p->type || !TY_ISUNKNOWN(p->type))) {
        assert(pos);
        (void)(err_dpos(pos, (glb)? ERR_LEX_LONGEID: ERR_LEX_LONGID) &&
               err_dpos(pos, ERR_LEX_LONGIDSTD, (long)((glb)? TL_ENAME_STD: TL_INAME_STD)) &&
               err_dpos(p->pos, ERR_LEX_SEEID, p->name));
        return 1;
    }

    return 0;
}


/*
 *  parses struct/union fields;
 *  ASSUMPTION: alignment factors must be a power of 2;
 *  ASSUMPTION: bit-field cannot straddle two storage units
 */
static int field(ty_t *ty)
{
    int unknown = 0;
    int inparamp = inparam;
    const lmap_t *posa[LEN];
    const lmap_t *pos,
                 *poscl;    /* ':' for bitfield */

    assert(ty);
    assert(ty_inttype);    /* ensures types initialized */
    assert(ir_cur);

    inparam = 0;
    {
        int n = 0;
        int plain, prt;

        while (clx_issdecl(CLX_TYLAF)) {
            ty_t *ty1 = specifier(NULL, posa, &plain, CLX_TYLAF);
            while (1) {
                tree_t *e;
                sym_field_t *p;
                const char *id = NULL;
                ty_t *fty;
                sx_t bitsize;

                if (!(clx_isadcl() || clx_tc == ':' || clx_tc == ';'))    /* ; for extension */
                    err_dpos(lmap_after(clx_ppos), ERR_PARSE_NODCLR, " for member");
                else {
                    fty = dclr(ty1, &id, NULL, 0, posa);
                    unknown |= fty->t.unknown;
                    assert(posa[PI] || posa[PD] || posa[PS]);
                    pos = posa[(posa[PI])? PI: (posa[PD])? PD: PS];
                    p = ty_newfield(id, ty, fty, pos);
                    if (clx_tc == ':') {
                        poscl = clx_cpos;
                        p->type = p->type->t.type;
                        if (!TY_ISINT(p->type) && !TY_ISUNSIGNED(p->type)) {
                            if (!TY_ISUNKNOWN(p->type)) {
                                assert(posa[PS]);
                                err_dmpos(poscl, ERR_PARSE_INVBITTYPE, posa[PS],
                                          (TY_ISPTR(p->type) || TY_ISFUNC(p->type) ||
                                           TY_ISARRAY(p->type))? posa[PD]: NULL, NULL);
                            }
                            p->type = ty_inttype;
                            plain = 0;    /* shuts up additional warnings */
                        }
                        clx_tc = clx_next();
                        bitsize = xI,
                            e = simp_intexpr(0, &bitsize, 0, xO, "bit-field size", NULL);
                        if (xgs(bitsize, xis(TG_CHAR_BIT*p->type->size)) || xls(bitsize, xO)) {
                            assert(e);
                            err_dpos(TREE_TW(e), ERR_PARSE_INVBITSIZE,
                                     (long)TG_CHAR_BIT*p->type->size);
                            bitsize = xis(TG_CHAR_BIT*p->type->size);
                        } else if (xe(bitsize, xO) && id) {
                            assert(posa[PI] && e);
                            err_dmpos(posa[PI], ERR_PARSE_EXTRAID, TREE_TW(e), NULL, id, "");
                            p->name = hash_int(sym_genlab(1));
                        } else if ((plain & 1) && id) {
                            assert(posa[PS]);
                            err_dmpos(poscl, ERR_PARSE_PINTFLD, posa[PS], NULL);
                            p->type = ty_qualc(p->type->op & TY_CONSVOL,
                                               (main_opt()->pfldunsign)? ty_unsignedtype:
                                                                         ty_inttype);
                        }
                        p->bitsize = xns(bitsize);
                        p->lsb = 1;
                    } else if ((main_opt()->std == 0 || main_opt()->std > 2) && !id &&
                               TY_ISSTRUNI(p->type)) {
                        if (!GENNAME(TY_UNQUAL(p->type)->u.sym->name))
                            (void)(err_dpos(lmap_after(pos), ERR_PARSE_NOFNAME) &&
                                   err_dpos(TY_UNQUAL(p->type)->u.sym->pos, ERR_PARSE_ANONYTAG));
                        p->name = NULL;
                    } else {
                        if (!id)
                            err_dpos((posa[PD])? posa[PD]: lmap_after(pos), ERR_PARSE_NOFNAME);
                        if (TY_ISFUNC(p->type))
                            err_dpos((posa[PD])? posa[PD]: pos, ERR_PARSE_INVFTYPE);
                        else if (p->type->size == 0) {
                            p->incomp = 1;
                            err_dpos(pos, ERR_PARSE_INCOMPMEM);
                        } else if (((prt = ty_hasproto(p->type)) & 1) == 0) {
                            assert(posa[PS] || posa[PD]);
                            err_dpos(posa[(prt >> 1)? PS: PD], ERR_PARSE_NOPROTO, id, " member");
                        }
                    }
                    if (TY_ISCONST(ty_arrelem(p->type)) || TY_HASCONST(p->type))
                        ty->u.sym->u.s.cfield = 1;
                    if (TY_ISVOLATILE(ty_arrelem(p->type)) || TY_HASVOLATILE(p->type))
                        ty->u.sym->u.s.vfield = 1;
                    if (n++ == TL_MBR_STD)
                        (void)(err_dpos(pos, ERR_PARSE_MANYMBR) &&
                               err_dpos(pos, ERR_PARSE_MANYMBRSTD, (long)TL_MBR_STD));
                }
                if (clx_tc != ',')
                    break;
                clx_tc = clx_next();
                if (clx_xtracomma(';', "member declarator", 0))
                    break;
            }
            sset_test(';', sset_field, NULL);
        }
    }
    {
        ssz_t off = 0;
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
            ty->size = SSZ_MAX & (~(ty->align - 1));
            err_dpos(ty->u.sym->pos, ERR_TYPE_BIGOBJADJ, ty->size);
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
    const lmap_t *poss = clx_cpos,    /* struct/union */
                 *post;

    clx_tc = clx_next();
    post = clx_cpos;    /* tag or { */
    if (clx_tc == LEX_ID) {
        tag = clx_tok;
        clx_tc = clx_next();
    } else
        tag = "";

    if (clx_tc == '{') {
        const lmap_t *posm = clx_cpos;
        if (*tag == '\0' && inparam)
            err_dpos(lmap_range(poss, post), ERR_PARSE_ATAGPARAM, op);
        if (strunilev++ == TL_STRCT_STD)
            (void)(err_dpos(clx_cpos, ERR_PARSE_MANYSTR) &&
                   err_dpos(clx_cpos, ERR_PARSE_MANYSTRSTD, (long)TL_STRCT_STD));
        ty = ty_newstruct(clx_tc, op, tag, post);
        ty->u.sym->f.defined = 1;
        clx_tc = clx_next();
        if (clx_issdecl(CLX_TYLAF))
            ty->t.unknown = field(ty);
        else if (clx_tc == '}')
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_NOFIELD, op);
        else
            err_dpos(lmap_pin(clx_cpos), ERR_PARSE_INVFIELD, op);
        sset_test('}', sset_strdef, posm);
        {    /* fixes size of typedef name completed after its definition */
            sz_t n;
            alist_t *pos;
            ALIST_FOREACH(n, pos, ty->u.sym->u.s.blist) {
                S(pos->data)->type->size = ty->size;
                S(pos->data)->type->align = ty->align;
            }
        }
        strunilev--;
        assert(strunilev >= 0);
    } else {
        if (*tag == '\0') {
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_NOTAG, op);
            post = poss;
        }
        ty = ty_newstruct(clx_tc, op, tag, post);
    }
    if (*tag && main_opt()->xref)
        sym_use(ty->u.sym, post);

    return ty;
}


/*
 *  parses an enum specifier
 */
static ty_t *enumdcl(void)
{
    const char *tag;
    ty_t *ty;
    const lmap_t *poss = clx_cpos,    /* enum */
                 *post;

    assert(ty_inttype);    /* ensures types initialized */

    clx_tc = clx_next();
    post = clx_cpos;    /* tag or { */
    if (clx_tc == LEX_ID) {
        tag = clx_tok;
        clx_tc = clx_next();
    } else
        tag = "";

    if (clx_tc == '{') {
        int n = 0;
        sx_t k = x_I;
        alist_t *idlist = NULL;
        const lmap_t *posm = clx_cpos;

        ty = ty_newstruct(clx_tc, TY_ENUM, tag, post);
        clx_tc = clx_next();
        if (clx_tc != LEX_ID)
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_ENUMID);
        else
            do {
                sym_t *p;
                const char *id = clx_tok;
                const lmap_t *pose = clx_cpos;    /* enumerator */
                if (clx_sym) {
                    if (!TY_ISUNKNOWN(clx_sym->type))
                        (void)(err_dpos(pose, (SYM_SAMESCP(clx_sym, sym_scope))?
                                                  ERR_PARSE_REDECL: ERR_PARSE_HIDEID,
                                        clx_sym, " an identifier") &&
                               err_dpos(clx_sym->pos, ERR_PARSE_PREVDECL));
                } else
                    decl_chkid(id, pose, sym_ident, 0);
                clx_tc = clx_next();
                if (clx_tc == '=') {
                    clx_tc = clx_next();
                    k = xO, simp_intexpr(0, &k, 1, xctu(TG_INT_MAX), "enum constant", NULL);
                } else {
                    if (xe(k, TG_INT_MAX)) {
                        err_dpos(pose, ERR_PARSE_ENUMOVER, id, "");
                        k = x_I;
                    }
                    xinc(k);
                }
                p = sym_new(SYM_KENUM, id, pose, ty,
                            (sym_scope < SYM_SPARAM)? strg_perm: strg_func, k);
                idlist = alist_append(idlist, p, strg_perm);
                if (n++ == TL_ENUMC_STD)
                    (void)(err_dpos(pose, ERR_PARSE_MANYEC) &&
                           err_dpos(pose, ERR_PARSE_MANYECSTD, (long)TL_ENUMC_STD));
                if (clx_tc != ',')
                    break;
                clx_tc = clx_next();
                if (clx_tc == '}')
                    err_dpos(clx_ppos, ERR_PARSE_ENUMCOMMA);
                else if (clx_xtracomma('}', "enumerator", 1))
                    break;
            } while(clx_tc == LEX_ID);
        if (clx_tc == ';') {
            err_dpos(clx_cpos, ERR_PARSE_ENUMSEMIC);
            clx_tc = clx_next();
        }
        sset_test('}', sset_enumdef, posm);
        ty->u.sym->u.idlist = alist_toarray(idlist, strg_perm);
        ty->u.sym->f.defined = 1;
    } else {
        if (*tag == '\0') {
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_NOTAG, TY_ENUM);
            post = poss;
        }
        ty = ty_newstruct(clx_tc, TY_ENUM, tag, post);
        if (clx_tc == ';')
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_EMPTYDECL);
    }
    ty->type = ty_inttype;
    ty->size = ty->type->size;
    ty->align = ty->type->align;
    if (*tag && main_opt()->xref)
        sym_use(ty->u.sym, post);

    return ty;
}


/*
 *  skips erroneous initializers
 */
static void skipinit(const char *msg)
{
    if (clx_tc != '=')
        return;
    if (msg)
        err_dpos(clx_cpos, ERR_PARSE_NOINIT, msg);
    clx_tc = clx_next();
    init_skip();
}


/*
 *  installs a parameter
 */
static sym_t *dclparam(int sclass, const char *id, ty_t *ty, const lmap_t *posa[], int n)
{
    sym_t *p;

    assert(id);
    assert(ty);
    assert(posa);
    assert(posa[PI] || posa[PD] || posa[PS]);

    if (TY_ISFUNC(ty))
        ty = ty_ptr(ty);
    else if (TY_ISARRAY(ty))
        ty = ty_atop(ty);

    if (sclass > 0 && sclass != LEX_REGISTER) {
        assert(posa[PC]);
        if (n == 0)
            err_dpos(posa[PC], ERR_PARSE_INVCLS, sclass);
        sclass = LEX_AUTO;
    }

    p = sym_lookup(id, sym_ident);
    if (p) {
        if (!TY_ISUNKNOWN(p->type)) {
            if (p->scope == sym_scope) {
                assert(posa[PI]);
                (void)(err_dpos(posa[PI], ERR_PARSE_REDECL, p, " an identifier") &&
                       err_dpos(p->pos, ERR_PARSE_PREVDECL));
                id = sym_semigenlab();    /* avoids type change of existing param */
            } else if (sclass != -1 && !GENSYM(p)) {
                assert(posa[PI]);
                (void)(err_dpos(posa[PI], ERR_PARSE_HIDEID, p, " an identifier") &&
                       err_dpos(p->pos, ERR_PARSE_PREVDECL));
            }
        }
    } else if (sclass != -1)
        decl_chkid(id, posa[PI], sym_ident, 0);
    if (sclass <= 0)
        sclass = LEX_AUTO;
    p = sym_new(SYM_KORDIN, id, posa[(posa[PI])? PI: (posa[PD])? PD: PS], sclass, ty, strg_func);
    p->f.wregister = (sclass == LEX_REGISTER);
    p->f.defined = 1;
    skipinit("parameter");

    return p;
}


#define ISSUEONCE(m, p)                    \
    do {                                   \
        if (!m)                            \
            err_dpos(p, ERR_PARSE_##m),    \
            m = 1;                         \
    } while(0)

/*
 *  parses parameters
 */
static node_t *parameter(ty_t *fty, const lmap_t *posm)    /* sym_t */
{
    alist_t *list = NULL;
    node_t *param;    /* sym_t */
    const lmap_t *posa[LEN];
    const lmap_t *posfs,    /* first declaration specifier */
                 *posl;     /* '...' if any */

    assert(fty);
    assert(posm);
    assert(ty_voidtype);    /* ensures types initialized */

    sym_enterscope();
    if (sym_scope > SYM_SPARAM)
        sym_enterscope();

    inparam++;
    while (1)    /* to handle mixed proto/non-proto */
        if (clx_isparam("") || clx_tc == LEX_ELLIPSIS || clx_ispdcl()) {
            int n = 0;
            ty_t *ty1 = NULL;
            int ellipsis = 0;    /* ellipsis seen */
            int VOIDALONE,    /* ERR_PARSE_VOIDALONE issued */
                ELLALONE,     /* ERR_PARSE_ELLALONE issued */
                ELLSEEN,      /* ERR_PARSE_ELLSEEN issued */
                QUALVOID;     /* ERR_PARSE_QUALVOID issued */
            VOIDALONE = ELLALONE = ELLSEEN = QUALVOID = 0;
            do {
                ty_t *ty;
                int sclass = 0;
                const char *id = NULL;
                if (clx_tc == LEX_ELLIPSIS) {
                    if (ty1 && !ellipsis) {
                        static sym_t sentinel;
                        posl = clx_cpos;
                        if (!sentinel.type)
                            sentinel.type = ty_voidtype;
                        if (ty1 == ty_voidtype)
                            ISSUEONCE(VOIDALONE, posfs);
                        else {
                            list = alist_append(list, &sentinel, strg_perm);
                            ellipsis = 1;
                        }
                    } else if (ellipsis)
                        ISSUEONCE(ELLSEEN, posl);
                    else    /* !ty1 */
                        ISSUEONCE(ELLALONE, clx_cpos);
                    clx_tc = clx_next();
                } else {
                    int dummy, prt;
                    int strunilevp = strunilev;
                    if (!clx_isparam("[,);")) {
                        err_dpos(lmap_after(clx_ppos), ERR_PARSE_NOPTYPE);
                        if (!clx_isadcl())    /* accepts declarators with default int */
                            break;
                    }
                    n++;
                    strunilev = 0;
                    ty = specifier(&sclass, posa, &dummy, CLX_TYLAP "[,);");
                    strunilev = strunilevp;
                    ty = dclr(ty, &id, NULL, 1, posa);
                    if (TY_ISVOID(ty) || ty1 == ty_voidtype) {
                        assert(posa[PS]);
                        if (TY_ISVOID(ty) && id)
                            err_dmpos(posa[PS], ERR_PARSE_VOIDID, posa[PI], NULL);
                        if (!ty1 && TY_ISQUAL(ty) && TY_ISVOID(ty) && !id)
                            ISSUEONCE(QUALVOID, posa[PS]);
                        if (ty1)
                            ISSUEONCE(VOIDALONE, lmap_pin(posa[PS]));
                    }
                    if (ellipsis)
                        ISSUEONCE(ELLSEEN, posl);
                    if (ty1 != ty_voidtype && !TY_ISVOID(ty) && !ellipsis) {
                        if (!id)
                            id = hash_int(n);
                        list = alist_append(list, dclparam(sclass, id, ty, posa, 0), strg_perm);
                        if (((prt = ty_hasproto(ty)) & 1) == 0) {
                            assert(posa[PS] || posa[PD]);
                            err_dpos(posa[(prt >> 1)? PS: PD], ERR_PARSE_NOPROTO,
                                     id, " parameter");
                        }
                    } else
                        skipinit("parameter");
                    if (!ty1) {
                        ty1 = TY_UNQUAL(ty)->t.type;
                        posfs = posa[PS];    /* may be NULL */
                    }
                }
                if (clx_tc != ',')
                    break;
                clx_tc = clx_next();
            } while(!clx_xtracomma(')', "parameter declaration", 0));
            fty->u.f.proto = ARENA_ALLOC(strg_perm,
                                         sizeof(*fty->u.f.proto)*(alist_length(list)+1));
            param = alist_toarray(list, strg_func);
            for (n = 0; param[n]; n++)
                fty->u.f.proto[n] = S(param[n])->type;
            fty->u.f.proto[n] = NULL;
            fty->u.f.oldstyle = 0;
            break;
        } else {
            if (clx_tc == LEX_ID || clx_tc == '=')    /* accepts erroneous initializer */
                while (1) {
                    sym_t *p;
                    if (clx_tc == LEX_ID) {
                        posa[PI] = clx_cpos;
                        p = dclparam(-1, clx_tok, ty_inttype, posa, 0);
                        p->f.defined = 0;    /* old-style */
                        list = alist_append(list, p, strg_perm);
                        clx_tc = clx_next();
                    } else
                        err_dpos(lmap_pin(clx_cpos), ERR_PARSE_PARAMID);
                    skipinit("parameter");
                    if (clx_tc != ',')
                        break;
                    clx_tc = clx_next();
                    if (clx_xtracomma(')', "parameter identifier", 0))
                        break;
                }
            if (clx_isparam("") || clx_tc == LEX_ELLIPSIS) {
                err_dpos(lmap_pin(clx_cpos), ERR_PARSE_MIXPROTO);
                list = NULL;
                continue;
            }
            param = alist_toarray(list, strg_func);
            fty->u.f.proto = NULL;
            fty->u.f.oldstyle = 1;
            break;
        }
    sset_test(')', sset_initb, posm);
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
static ty_t *dclr1(const char **id, node_t **param,                /* sym_t */
                   int abstract, int lev, const lmap_t **pposi)
{
    ty_t *ty = NULL;
    const lmap_t *posm;

    switch(clx_tc) {
        case LEX_ID:
            if (pposi)
                *pposi = clx_cpos;
            if (!id)
                err_dpos(clx_cpos, ERR_PARSE_EXTRAID, clx_tok, "");
            else
                *id = clx_tok;
            clx_tc = clx_next();
            break;
        case '*':
            clx_tc = clx_next();
            if (clx_tc == LEX_CONST || clx_tc == LEX_VOLATILE) {
                ty_t *ty1;
                ty1 = ty = tnode(clx_tc, NULL);
                while ((clx_tc = clx_next()) == LEX_CONST || clx_tc == LEX_VOLATILE)
                    ty1 = tnode(clx_tc, ty1);
                ty->type = dclr1(id, param, abstract, lev, pposi);
                ty = ty1;
            } else
                ty = dclr1(id, param, abstract, lev, pposi);
            ty = tnode(TY_POINTER, ty);
            break;
        case '(':
            posm = clx_cpos;
            clx_tc = clx_next();
            if (abstract && (clx_isparam("") || clx_tc == ')')) {
                node_t *arg;    /* sym_t */
                ty = tnode(TY_FUNCTION, ty);
                arg = parameter(ty, posm);
                exitparam(arg);
            } else {
                if (!clx_isdcl())
                    goto fparam;
                if (lev == TL_PAREND_STD)
                    (void)(err_dpos(clx_ppos, ERR_PARSE_MANYPD) &&
                           err_dpos(clx_ppos, ERR_PARSE_MANYPDSTD, (long)TL_PAREND_STD));
                ty = dclr1(id, param, abstract, lev+1, pposi);
                sset_expect(')', posm);
                if (abstract && !ty && (!id || !*id))
                    return tnode(TY_FUNCTION, NULL);
            }
            break;
        default:
            break;
    }
    while (clx_tc == '(' || clx_tc == '[')
        switch(clx_tc) {
            case '(':
                posm = clx_cpos;
                clx_tc = clx_next();
            fparam:
                {
                    node_t *arg;    /* sym_t */
                    ty = tnode(TY_FUNCTION, ty);
                    arg = parameter(ty, posm);
                    if (param && !*param)
                        *param = arg;
                    else
                        exitparam(arg);
                }
                break;
            case '[':
                posm = clx_cpos;
                clx_tc = clx_next();
                {
                    tree_t *e;
                    sx_t n = xO;
                    if (clx_isexpr()) {
                        n = xI, e = simp_intexpr(']', &n, 1, xiu(SSZ_MAX), "array size", posm);
                        if (xles(n, xO)) {
                            assert(e);
                            err_dpos(TREE_TW(e), ERR_PARSE_INVARRSIZE);
                            n = xI;
                        }
                    } else
                        sset_expect(']', posm);
                    ty = tnode(TY_ARRAY, ty);
                    ty->size = xns(n);
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
static ty_t *dclr(ty_t *basety, const char **id, node_t **param,    /* sym_t */
                  int abstract, const lmap_t *posa[])
{
    int n = 0;
    ty_t *ty;
    const lmap_t *pos;

    assert(basety);

    posa[PD] = (clx_isadcl())? clx_cpos: NULL;
    posa[PI] = NULL;
    ty = dclr1(id, param, abstract, 0, &posa[PI]);
    if (posa[PD])
        posa[PD] = lmap_range(posa[PD], clx_ppos);
    for (; ty; ty = ty->type)
        switch(ty->op) {
            case TY_POINTER:
                basety = ty_ptr(basety);
                n++;
                break;
            case TY_FUNCTION:
                basety = ty_func(basety, ty->u.f.proto, ty->u.f.oldstyle, posa[PD]);
                n++;
                break;
            case TY_ARRAY:
                basety = ty_array(basety, ty->size, posa[PD]);
                n++;
                break;
            case TY_CONST:
            case TY_VOLATILE:
                basety = ty_qual(ty->op, basety, 1, posa[PD]);
                break;
            default:
                assert(!"invalid type operator -- should never reach here");
                break;
        }
    if (n > TL_DECL_STD) {
        assert(posa[PD]);
        (void)(err_dpos(posa[PD], ERR_PARSE_MANYDECL) &&
               err_dpos(posa[PD], ERR_PARSE_MANYDECLSTD, (long)TL_DECL_STD));
    }
    if (basety->size > TL_OBJ_STD) {    /* note TL_OBJ_STD is unsigned */
        pos = posa[(n > 0)? PD: PS];
        assert(pos);
        (void)(err_dpos(pos, ERR_TYPE_BIGOBJ) &&
               err_dpos(pos, ERR_TYPE_BIGARRSTD, (unsigned long)TL_OBJ_STD));
    }

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
static void initglobal(sym_t *p, const lmap_t *pos, int tolit)
{
    ty_t *ty;

    assert(p);
    assert(pos);
    assert(clx_tc != '=');

    if (p->sclass == LEX_STATIC) {
        ty = ty_arrelem(p->type);
        decl_defglobal(p, (tolit || TY_ISCONST(ty))? INIT_SEGLIT: INIT_SEGDATA);
    } else
        decl_defglobal(p, INIT_SEGDATA);
    ty = init_init(p->type, 0, pos);
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
}


/*
 *  installs a typedef name
 */
static sym_t *typedefsym(const char *id, ty_t *ty, const lmap_t *posd, int plain)
{
    sym_t *p;

    assert(id);
    assert(ty);
    assert(posd);

    p = sym_lookup(id, sym_ident);
    if (p) {
        if (!TY_ISUNKNOWN(p->type))
            (void)(err_dpos(posd, (SYM_SAMESCP(p, sym_scope))? ERR_PARSE_REDECL: ERR_PARSE_HIDEID,
                            p, " an identifier") &&
                   err_dpos(p->pos, ERR_PARSE_PREVDECL));
    } else
        decl_chkid(id, posd, sym_ident, 0);
    ty = memcpy(ARENA_ALLOC(strg_perm, sizeof(*ty)), ty, sizeof(*ty));
    ty->t.name = id;
    ty->t.plain = plain;
    p = sym_new(SYM_KTYPEDEF, id, posd, ty, (sym_scope < SYM_SPARAM)? strg_perm: strg_func);
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
                (void)(err_dpos(lmap_pin(p->pos), ERR_PARSE_ENUMINT) &&
                       err_dpos(lmap_pin(pt->pos), ERR_PARSE_PREVDECL));
        } else
            (void)(err_dpos(p->pos, ERR_PARSE_REDECLTYW, p, " an identifier", p->type, pt->type) &&
                   err_dpos(pt->pos, ERR_PARSE_PREVDECL));
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
static sym_t *dclglobal(int sclass, const char *id, ty_t *ty, const lmap_t *posa[], int n)
{
    sym_t *p;
    const lmap_t *posi;
    int visible = 0, eqret;

    assert(sclass != LEX_TYPEDEF);
    assert(id);
    assert(!GENNAME(id));
    assert(ty);
    assert(posa);
    assert(posa[PD]);
    assert(ir_cur);

    if (sclass && sclass != LEX_EXTERN && sclass != LEX_STATIC) {
        assert(posa[PC]);
        if (n == 0)
            err_dpos(posa[PC], ERR_PARSE_INVCLS, sclass);
        sclass = 0;
    }
    if (!sclass)
        sclass = (TY_ISFUNC(ty))? LEX_EXTERN: LEX_AUTO;

    posi = (posa[PI])? posa[PI]: posa[PD];
    p = sym_lookup(id, sym_global);
    if (p) {
        visible = 1;
        if (p->sclass != LEX_TYPEDEF && p->sclass != LEX_ENUM) {
            if ((eqret = ty_equiv(ty, p->type, 1)) != 0) {
                if (eqret > 1)
                    (void)(err_dpos(lmap_pin(posi), ERR_PARSE_ENUMINT) &&
                           err_dpos(lmap_pin(p->pos), ERR_PARSE_PREVDECL));
                ty = ty_compose(ty, p->type);
            } else if (!TY_ISUNKNOWN(p->type)) {
                assert(sym_scope == SYM_SGLOBAL || sym_scope == SYM_SPARAM);
                (void)(err_dpos(posi, ERR_PARSE_REDECLTY, p, " an identifier", ty, p->type) &&
                       err_dpos(p->pos, ERR_PARSE_PREVDECL));
            }
        } else if (!TY_ISUNKNOWN(p->type)) {
            assert(sym_scope == SYM_SGLOBAL || sym_scope == SYM_SPARAM);
            (void)(err_dpos(posi, ERR_PARSE_REDECL, p, " an identifier") &&
                   err_dpos(p->pos, ERR_PARSE_PREVDECL));
        }
        if (!TY_ISFUNC(ty) && p->f.defined && clx_tc == '=' && eqret)
            (void)(err_dpos(clx_cpos, ERR_PARSE_REDEF, p, " an identifier") &&
                   err_dpos(p->pos, ERR_PARSE_PREVDEF));
        if ((p->sclass == LEX_EXTERN && sclass == LEX_STATIC) ||
            (p->sclass == LEX_STATIC && sclass == LEX_AUTO) ||
            (p->sclass == LEX_AUTO && sclass == LEX_STATIC)) {
            const char *before, *after;
            if (p->sclass == LEX_STATIC)
                before = "non-", after = "";
            else
                before = "", after = "non-";
            (void)(err_dpos(posa[(posa[PC])? PC: (posa[PS])? PS: PD], ERR_PARSE_INVLINK, before,
                            p, " an identifier", after) &&
                   err_dpos(lmap_pin(p->pos), ERR_PARSE_PREVDECL));
        }
        if (p->sclass == LEX_EXTERN ||
            p->sclass == LEX_TYPEDEF)    /* to pass to symgsc() */
            p->sclass = sclass;
    } else {
        p = sym_new(SYM_KGLOBAL, id, posi, sclass, ty);
        if (sclass != LEX_STATIC) {
            static int nglobal;
            if (nglobal++ == TL_NAME_STD)
                (void)(err_dpos(posi, ERR_PARSE_MANYEID) &&
                       err_dpos(posi, ERR_PARSE_MANYEIDSTD, (long)TL_NAME_STD));
            if (!decl_chkid(id, posi, sym_global, 1))
                decl_chkid(id, posi, sym_extern, 1);
        } else
            decl_chkid(id, posi, sym_ident, 0);
    }
    p->type = ty;
    p->pos = posi;
    ir_cur->symgsc(p);
    {
        sym_t *q = sym_lookup(p->name, sym_extern);
        if (q) {
            if (!visible && p->sclass == LEX_STATIC) {
                assert(posa[PC]);
                (void)(err_dpos(posa[PC], ERR_PARSE_INVLINKW, p, " an identifier",
                                "static", "extern") &&
                       err_dpos(lmap_pin(q->pos), ERR_PARSE_PREVDECL));
            }
            cmptylist(p, q);
        }
    }
    if (clx_tc != '=') {
        if (p->sclass == LEX_STATIC && !TY_ISFUNC(p->type) && p->type->size == 0)
            err_dpos(p->pos, ERR_PARSE_INCOMPTYPE, p, " an identifier");
    } else if (TY_ISFUNC(p->type))
        skipinit("function");
    else if (TY_ISVOID(p->type)) {
        p->f.defined = 1;    /* stops diagnostics from doglobal() */
        skipinit("`void' type");
    } else {
        clx_tc = clx_next();
        initglobal(p, clx_ppos, 0);
    }

    return p;
}


/*
 *  installs a local identifier
 */
static sym_t *dcllocal(int sclass, const char *id, ty_t *ty, const lmap_t *posa[], int n)
{
    int eqret;
    sym_t *p, *q, *r;

    assert(sclass != LEX_TYPEDEF);
    assert(id);
    assert(ty);
    assert(posa);
    assert(posa[PS] && posa[PD]);
    assert(pregvar || err_count() > 0);
    assert(pautovar || err_count() > 0);
    assert(ir_cur);
    UNUSED(n);

    if (!sclass)
        sclass = (TY_ISFUNC(ty))? LEX_EXTERN: LEX_AUTO;
    else if (TY_ISFUNC(ty) && sclass != LEX_EXTERN) {
        assert(posa[PC]);
        err_dpos(posa[PC], ERR_PARSE_INVCLSID, sclass, ty, id, "");
        sclass = LEX_EXTERN;
    }

    if (sclass != LEX_EXTERN)    /* cannot determine linkage here */
        decl_chkid(id, posa[PI], sym_ident, 0);
    q = sym_lookup(id, sym_ident);
    if (q && !TY_ISUNKNOWN(q->type)) {
        assert(posa[PI]);
        if (SYM_SAMESCP(q, sym_scope)) {
            if (!(q->sclass == LEX_EXTERN && sclass == LEX_EXTERN &&
                  (eqret = ty_equiv(ty, q->type, 1)) != 0))
                (void)(err_dpos(posa[PI], ERR_PARSE_REDECL, q, " an identifier") &&
                       err_dpos(q->pos, ERR_PARSE_PREVDECL));
            else if (eqret > 1)
                (void)(err_dpos(lmap_pin(posa[PI]), ERR_PARSE_ENUMINT) &&
                       err_dpos(lmap_pin(q->pos), ERR_PARSE_PREVDECL));
        } else if (sclass != LEX_EXTERN || !LINKEDID(q))
            (void)(err_dpos(posa[PI], ERR_PARSE_HIDEID, q, " an identifier") &&
                   err_dpos(q->pos, ERR_PARSE_PREVDECL));
    }
    assert(sym_scope >= SYM_SLOCAL);
    p = sym_new(SYM_KORDIN, id, posa[(posa[PI])? PI: PD], sclass, ty, strg_func);
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
                if (!decl_chkid(p->name, posa[PI], sym_global, 1))
                    decl_chkid(p->name, posa[PI], sym_extern, 1);
                ir_cur->symgsc(p);
            }
            if (q && q->scope != sym_scope) {
                if ((eqret = ty_equiv(p->type, q->type, 1))) {
                    if (eqret > 1)
                        (void)(err_dpos(lmap_pin(p->pos), ERR_PARSE_ENUMINT) &&
                               err_dpos(lmap_pin(q->pos), ERR_PARSE_PREVDECL));
                    p->type = ty_compose(q->type, p->type);
                } else
                    (void)(err_dpos(p->pos, ERR_PARSE_REDECLTY, p, " an identifier",
                                    p->type, q->type) &&
                           err_dpos(q->pos, ERR_PARSE_PREVDECL));
            } else if (!q && r) {
                if ((eqret = ty_equiv(p->type, r->type, 1)) == 0)
                    (void)(err_dpos(p->pos, ERR_PARSE_REDECLTYW, p, " an identifier",
                                    p->type, r->type) &&
                           err_dpos(r->pos, ERR_PARSE_PREVDECL));
                else if (eqret > 1)
                    (void)(err_dpos(lmap_pin(p->pos), ERR_PARSE_ENUMINT) &&
                           err_dpos(lmap_pin(r->pos), ERR_PARSE_PREVDECL));
                if (r->sclass == LEX_STATIC)
                    (void)(err_dpos(posa[(posa[PC])? PC: PS], ERR_PARSE_INVLINKW,
                                    p, " an identifier", "extern", "static") &&
                           err_dpos(lmap_pin(r->pos), ERR_PARSE_PREVDECL));
            }
            r = sym_lookup(p->name, sym_extern);
            if (r && q != r)
                cmptylist(p, r);
            else {
                r = sym_new(SYM_KEXTERN, p->name, p->pos, p->sclass, p->type);
                r->tylist = sym_tylist(r->tylist, p);
            }
            break;
        case LEX_STATIC:
            ir_cur->symgsc(p);
            if (clx_tc == '=') {
                clx_tc = clx_next();
                initglobal(p, clx_ppos, 0);
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
    if (clx_tc == '=') {
        tree_t *e;
        const lmap_t *pos = clx_cpos;
        if (TY_ISFUNC(p->type) || sclass == LEX_EXTERN)
            skipinit((TY_ISFUNC(p->type))? "function": "local extern");
        else {
            clx_tc = clx_next();
            if (!TY_ISUNKNOWN(p->type) && !TY_ISVOID(p->type)) {
                sym_ref(p, 1);
                stmt_defpoint(NULL);
                if (TY_ISSCALAR(p->type) || (TY_ISSTRUNI(p->type) && clx_tc != '{')) {
                    if (clx_tc == '{') {
                        const lmap_t *posm = clx_cpos;
                        clx_tc = clx_next();
                        e = expr_asgn(0, 0, 1, NULL);
                        if (clx_tc == ',')
                            clx_tc = clx_next();
                        clx_xtracomma(',', "initializer", 1);
                        sset_expect('}', posm);
                        if (e)
                            e->pos = tree_npos(posm, posm, clx_ppos);    /* includes braces */
                    } else
                        e = expr_asgn(0, 0, 1, NULL);
                } else {
                    sym_t *t1;
                    const lmap_t *poss = clx_cpos;
                    t1 = sym_new(SYM_KGEN, LEX_STATIC, p->type, SYM_SGLOBAL);
                    initglobal(t1, pos, 1);
                    if (TY_ISARRAY(p->type) && p->type->size == 0 && t1->type->size > 0)
                        p->type = ty_array(p->type->type, t1->type->size/t1->type->type->size,
                                           NULL);
                    e = tree_id(t1, tree_npos1(lmap_range(poss, clx_ppos)));
                }
                if (p->type->size > 0) {
                    dag_walk(tree_asgnid(p, e, tree_npos(p->pos, pos, TREE_NR(e))), 0, 0);
                    p->f.set = 1;
                }
            } else
                init_skip();
        }
    }
    if (!TY_ISFUNC(p->type) && p->f.defined && p->type->size == 0)
        err_dpos(p->pos, ERR_PARSE_INCOMPTYPE, p, " an identifier");

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
        err_dpos(p->pos, ERR_PARSE_VOIDOBJ, p, "");
    if (p->scope >= SYM_SPARAM && TY_ISVOLATILE(p->type))    /* P, L */
        p->f.addressed = 1;
    if (!GENNAME(p->name) && p->f.defined && p->ref == 0 && !TY_ISVOID(p->type) &&
        !err_experr()) {    /* P, L, F */
        if (p->sclass == LEX_STATIC)
            err_dpos(p->pos, ERR_PARSE_REFSTATIC, p, " identifier");
        else if (p->scope == SYM_SPARAM)
            err_dpos(p->pos, ERR_PARSE_REFPARAM, p, "");
        else if (p->scope >= SYM_SLOCAL && p->sclass != LEX_EXTERN)
            err_dpos(p->pos, ERR_PARSE_REFLOCAL, p, " identifier");
        p->f.reference = 1;
    }
    if (p->f.set == 1 && !p->f.reference && !err_experr()) {    /* P, L, F */
        if (p->sclass == LEX_STATIC)
            err_dpos(p->pos, ERR_PARSE_SETNOREFS, p, " identifier");
        else if (p->scope == SYM_SPARAM)
            err_dpos(p->pos, ERR_PARSE_SETNOREFP, p, "");
        else if (p->scope >= SYM_SLOCAL && p->sclass != LEX_EXTERN)
            err_dpos(p->pos, ERR_PARSE_SETNOREFL, p, " identifier");
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
        err_dpos(p->pos, ERR_PARSE_UNDSTATIC, p, " identifier");
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
        if (TY_ISARRAY(p->type) && p->type->size == 0 && p->type->type->size > 0)
            p->type = ty_array(p->type->type, 1, p->pos);
        if (p->type->size > 0) {
            decl_defglobal(p, INIT_SEGBSS);
            ir_cur->initspace(p->type->size);
        } else if (p->sclass != LEX_STATIC)    /* static incomplete checked in dclglobal() */
            err_dpos(p->pos, ERR_PARSE_INCOMPTYPE, p, " an identifier");
        p->f.defined = 1;
    }
    if (main_opt()->proto && !TY_ISFUNC(p->type) && !GENSYM(p) && p->f.defined) {
        int anonym;
        fprintf(stderr, "%s;\n", ty_outdecl(p->type, p->name, &anonym, 0));
        if (anonym)
            err_dpos(p->pos, ERR_TYPE_ERRPROTO, p, " a variable");
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
            ir_cur->initstr(p->type->size, xctp(p->u.c.v.p));
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
        err_dpos(p->pos, ERR_STMT_UNUSEDLAB, p, "");
    } else if (!p->f.defined)
        err_dpos(p->pos, ERR_STMT_UNDEFLAB, p, "");
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
    const lmap_t *pos,     /* statement */
                 *posm;
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
    posm = sset_expect('{', NULL);
    pos = clx_cpos;
    stmtseen = 0;
    do {
        if (clx_ispdecl()) {
            if (main_opt()->std == 1 && stmtseen == 1)
                err_dpos(lmap_pin(clx_cpos), ERR_PARSE_MIXDCLSTMT);
            stmt_chkreach();
            pautovar = &autovar;
            pregvar = &regvar;
            do {
                decl(dcllocal, CLX_TYLA);
            } while(clx_ispdecl());
        }
        if (clx_ispstmt()) {
            stmtseen++;
            do {
                stmt_stmt(loop, swp, lev, pos, NULL, 0);
            } while(clx_ispstmt());
        }
        if (!clx_issdecl(CLX_TYLA) && clx_tc != '}' && clx_tc != LEX_EOI) {
            err_dpos(lmap_pin(clx_cpos), ERR_PARSE_INVDCLSTMT);
            sset_skip('}', sset_declb);
        }
    } while(clx_issdecl(CLX_TYLA) || clx_issstmt());
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
    sym_exitscope(posm);
}


/*
 *  exits a parameter scope
 */
static void exitparam(node_t param[])    /* sym_t */
{
    assert(param);

    if (param[0] && !S(param[0])->f.defined)
        err_dpos(lmap_pin(S(param[0])->pos), ERR_PARSE_EXTRAPARAM);
    if (sym_scope > SYM_SPARAM)
        sym_exitscope(NULL);
    sym_exitscope(NULL);
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
        /* cannot assert(p->sclass != CLX_TYPEDEF) due to unknown type (#59) */
        if (p->name == S(callee[i])->name) {
            callee[i] = p;
            return;
        }
    }
    if (*p->name != '#' && !TY_ISUNKNOWN(p->type))
        err_dpos(p->pos, ERR_PARSE_NOPARAM, p, "");
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
        err_dpos(decl_cfunc->pos, ERR_XTRA_INVMAIN, ty, decl_cfunc->name, &n);
}


/*
 *  parses a function definition
 */
static void funcdefn(int sclass, const char *id, ty_t *ty, node_t param[],    /* sym_t */
                     const lmap_t *posa[])
{
    int i, n;
    int eqret;
    ty_t *rty;
    sym_t *p;
    const lmap_t *posm;
    node_t *callee, *caller;    /* sym_t */

    assert(id);
    assert(ty);
    assert(param);
    assert(posa);
    assert(posa[PD] && posa[PI]);
    assert(TY_ISFUNC(ty));
    assert(ty_inttype);    /* ensures types initialized */
    assert(ir_cur);

    posm = (clx_tc == '{')? clx_cpos: NULL;
    rty = ty_freturn(ty);
    assert(!TY_ISFUNC(rty) && !TY_ISARRAY(rty));
    if (TY_ISSTRUNI(rty) && rty->size == 0) {
        assert(posa[PS]);
        err_dpos(posa[PS], ERR_PARSE_INCOMPRET);
    }
    if (TY_ISQUAL(rty)) {
        const lmap_t *pos = (TY_ISPTR(rty) && !rty->t.name && !TY_UNQUAL(rty)->t.name)?
                                posa[PD]: posa[PS];
        assert(pos);
        err_dpos(pos, ERR_PARSE_QUALFRET);
    }
    for (n = 0; param[n]; n++)
        continue;
    if (n > 0 && !S(param[n-1])->name)
        param[--n] = NULL;
    if (n > TL_PARAM_STD)
        (void)(err_dpos(S(param[TL_PARAM_STD])->pos, ERR_PARSE_MANYPARAM) &&
               err_dpos(S(param[TL_PARAM_STD])->pos, ERR_PARSE_MANYPSTD, (long)TL_PARAM_STD));
    if (ty->u.f.oldstyle) {
        const lmap_t *pos;
        node_t *proto = NULL;    /* ty_t */

        p = sym_lookup(id, sym_global);
        if (p && TY_ISFUNC(p->type) && p->f.defined && ty_equiv(ty, p->type, 1))
            (void)(err_dpos(posa[PI], ERR_PARSE_REDEF, p, " an identifier") &&
                   err_dpos(p->pos, ERR_PARSE_PREVDEF));
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
        while (clx_tc != '{' && clx_tc != LEX_EOI && clx_issdecl(CLX_TYLAN)) {
            if (clx_issdecl(CLX_TYLAN))
                decl(dclparam, CLX_TYLAN);
            else {
                err_dpos(lmap_pin(clx_cpos), ERR_PARSE_INVDECL);
                sset_skip('{', sset_initb);
                if (clx_tc == ';')    /* avoids infinite loop */
                    clx_tc = clx_next();
            }
        }
        sym_foreach(sym_ident, SYM_SPARAM, oldparam, callee);
        for (i = 0; (p = callee[i]) != NULL; i++) {
            if (!p->f.defined) {
                const lmap_t *posa[LEN];
                posa[PI] = p->pos;
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
                    (void)(err_dpos(lmap_pin(S(caller[i])->pos), ERR_PARSE_ENUMINT) &&
                           err_dpos(lmap_pin(pos), ERR_PARSE_PREVDECL));
            }
            if (caller[i])
                (void)(err_dpos(S(caller[i])->pos, ERR_PARSE_PARAMMATCH) &&
                       err_dpos(lmap_pin(pos), ERR_PARSE_PREVDECL));
            else if (proto[i])
                (void)(err_dpos(posa[PD], ERR_PARSE_PARAMMATCH) &&
                       err_dpos(lmap_pin(pos), ERR_PARSE_PREVDECL));
        } else {
            proto = ARENA_ALLOC(strg_perm, (n+1)*sizeof(*proto));
            err_dpos(posa[PD], ERR_PARSE_NOPROTO, id, " function definition");
            for (i = 0; i < n; i++)
                proto[i] = S(caller[i])->type;
            proto[i] = NULL;
            decl_cfunc->type = ty_func(rty, proto, 1, posa[PD]);
            decl_cfunc->type->u.f.implint = ty->u.f.implint;
        }
        inparam--;
        assert(inparam >= 0);
    } else {
        callee = param;
        caller = ARENA_ALLOC(strg_func, (n+1)*sizeof(*caller));
        for (i = 0; (p = callee[i]) != NULL && p->name; i++) {
            if (p->name == id && !sym_lookup(id, sym_global))
                (void)(err_dpos(p->pos, ERR_PARSE_HIDEID, p, " an identifier") &&
                       err_dpos(posa[PI], ERR_PARSE_PREVDECL));
            caller[i] = ARENA_ALLOC(strg_func, sizeof(*S(caller[i])));
            *S(caller[i]) = *p;
            S(caller[i])->type = ty_ipromote(p->type);
            S(caller[i])->sclass = LEX_AUTO;
            if (GENNAME(p->name))
                err_dpos(p->pos, ERR_PARSE_NOPARAMID, i+1);
        }
        caller[i] = NULL;
        p = sym_lookup(id, sym_ident);
        if (p && TY_ISFUNC(p->type) && p->f.defined && ty_equiv(ty, p->type, 1))
            (void)(err_dpos(posa[PI], ERR_PARSE_REDEF, p, " an identifier") &&
                   err_dpos(p->pos, ERR_PARSE_PREVDEF));
        decl_cfunc = dclglobal(sclass, id, ty, posa, 0);
    }

    for (i = 0; (p = callee[i]) != NULL; i++)
        if (p->type->size == 0) {
            err_dpos(p->pos, ERR_PARSE_INCOMPARAM, p, "");
            S(caller[i])->type = p->type = ty_inttype;
        }
    posm = (clx_tc == '{')? clx_cpos: NULL;
    decl_cfunc->u.f.label = sym_genlab(1);
    decl_cfunc->u.f.pt = clx_cpos;    /* { or start of body */
    decl_cfunc->f.defined = 1;
    decl_callee = callee;
    if (sclass != LEX_STATIC && strcmp(id, "main") == 0) {
        decl_mainfunc = 1;
        chkmain();
    }
    if (main_opt()->xref)
        sym_use(decl_cfunc, posa[PI]);
    if (main_opt()->proto) {
        int anonym;
        fprintf(stderr, "%s;\n", ty_outdecl(decl_cfunc->type, id, &anonym, 0));
        if (anonym)
            err_dpos(posa[PI], ERR_TYPE_ERRPROTO, err_idsym(id), " a function");
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
                err_dpos(clx_cpos, ERR_STMT_NORETURN);
            stmt_retcode(NULL, clx_cpos);
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
    sym_exitscope(posm);
    sset_expect('}', posm);
    sym_label = stmt_lab = NULL;
    clear_declobj();
    err_cleareff();
}


/*
 *  parses declarations
 */
static void decl(sym_t *(*dcl)(int, const char *, ty_t *, const lmap_t *[], int),
                 const char *tyla)
{
    int n = 0;
    ty_t *ty, *ty1;
    int impl, sclass, prt;
    const lmap_t *posa[LEN];

    assert(dcl);

    strunilev = 0;
    ty = specifier(&sclass, posa, &impl, tyla);
    if (clx_isadcl()) {
        const char *id = NULL;
        if (sym_scope == SYM_SGLOBAL) {
            node_t *param = NULL;    /* sym_t */
            ty1 = dclr(ty, &id, &param, 0, posa);
            if (param && id && TY_ISFUNC(ty1) &&
                (clx_tc == '{' ||
                 (param[0] && !S(param[0])->f.defined && clx_issdecl(CLX_TYLAN)))) {
                if (sclass == LEX_TYPEDEF) {
                    assert(posa[PC]);
                    err_dpos(posa[PC], ERR_PARSE_TYPEDEFF);
                    sclass = LEX_EXTERN;
                }
                if (ty1->u.f.oldstyle) {
                    sym_exitscope(NULL);
                    sym_enterscope();
                }
                ty1->u.f.implint = (impl >> 1);
                funcdefn(sclass, id, ty1, param, posa);
                return;
            } else if (param)
                exitparam(param);
        } else
            ty1 = dclr(ty, &id, NULL, 0, posa);
        while (1) {
            if (posa[PD]) {
                if (!id) {
                    err_dpos(posa[PD], ERR_PARSE_NOID);
                    skipinit(NULL);
                } else {
                    assert(posa[PI]);
                    if (((prt = ty_hasproto(ty1)) & 1) == 0) {
                        assert(posa[PS] || posa[PD]);
                        err_dpos(posa[(prt >> 1)? PS: PD], ERR_PARSE_NOPROTO, id, " identifier");
                    }
                    if (sclass == LEX_TYPEDEF && dcl != dclparam)
                        typedefsym(id, ty1, posa[PI], impl & 1);
                    else
                        dcl(sclass, id, ty1, posa, n++);
                }
            }
            if (clx_tc != ',')
                break;
            clx_tc = clx_next();
            if (clx_xtracomma(';', "declarator", 0))
                break;
            id = NULL;
            if (clx_isadcl())
                ty1 = dclr(ty, &id, NULL, 0, posa);
            else {
                err_dpos(lmap_after(clx_ppos), ERR_PARSE_NODCLR, "");
                posa[PD] = NULL;
                skipinit(NULL);
            }
        }
    } else {
        if (sclass) {
            assert(posa[PC]);
            err_dpos(posa[PC], ERR_PARSE_NOUSECLS, sclass);
        }
        if (!TY_ISENUM(ty) && (!TY_ISSTRUNI(ty) || GENNAME(TY_UNQUAL(ty)->u.sym->name)))
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_EMPTYDECL);
        else if (inparam)
            err_dpos(lmap_after(clx_ppos), ERR_PARSE_DECLPARAM);
        else if (clx_tc == '=')
            err_dpos(clx_cpos, ERR_PARSE_UNUSEDINIT);
        skipinit(NULL);
    }
    sset_test(';', (sym_scope >= SYM_SPARAM)? sset_declb: sset_declf, NULL);
}


/*
 *  parses a translation unit
 */
void (decl_program)(void)
{
    sym_scope = SYM_SGLOBAL;

    if (clx_tc != LEX_EOI) {
        do {
            if (clx_issdecl(CLX_TYLA) || clx_isdcl()) {
                if (clx_isdcl() && !clx_istype(CLX_TYLA))
                    err_dpos(lmap_pin(clx_cpos), ERR_PARSE_NODECLSPEC);
                decl(dclglobal, CLX_TYLA);
                ARENA_FREE(strg_stmt);
                if (!main_opt()->glevel && !main_opt()->xref)
                    ARENA_FREE(strg_func);
            } else if (clx_tc == ';') {
                err_dpos(CLX_PCPOS(), ERR_PARSE_EMPTYDECL);
                clx_tc = clx_next();
            } else {
                err_dpos(lmap_pin(clx_cpos), ERR_PARSE_INVDECL);
                sset_skip(0, sset_decl);
                if (clx_tc == ';')    /* avoids "empty declaration" warning */
                    clx_tc = clx_next();
            }
        } while(clx_tc != LEX_EOI);
    } else
        err_dpos(clx_cpos, ERR_INPUT_EMPTYFILE);
}


/*
 *  parses a type name
 */
ty_t *(decl_typename)(const char *tyla)
{
    ty_t *ty;
    const lmap_t *posa[LEN];
    int dummy, prt;

    ty = specifier(NULL, posa, &dummy, tyla);
    if (clx_tc == '*' || clx_tc == '(' || clx_tc == '[') {
        ty = dclr(ty, NULL, NULL, 1, posa);
        if (((prt = ty_hasproto(ty)) & 1) == 0) {
            assert(posa[PS] || posa[PD]);
            err_dpos(posa[(prt >> 1)? PS: PD], ERR_PARSE_NOPROTO, NULL, " type name");
        }
    }

    return ty;
}


/*
 *  consumes an erroneous local declaration
 */
void (decl_errdecl)(const char *tyla)
{
    assert(err_count() > 0);
    decl(dcllocal, tyla);
}

/* end of decl.c */
