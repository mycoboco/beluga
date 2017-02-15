/*
 *  symbol table
 */

#include <limits.h>        /* LONG_MAX, CHAR_BIT */
#include <stdarg.h>        /* va_list, va_start, va_arg, va_end */
#include <stddef.h>        /* NULL, size_t */
#include <stdio.h>         /* sprintf */
#include <cbl/assert.h>    /* assert */
#include <cbl/arena.h>     /* arena_t, ARENA_CALLOC, ARENA_ALLOC */
#include <cdsl/hash.h>     /* hash_new, hash_int, hash_string */

#include "alist.h"
#include "clx.h"
#include "common.h"
#include "err.h"
#include "ir.h"
#include "lex.h"
#include "lmap.h"
#include "main.h"
#include "op.h"
#include "strg.h"
#include "ty.h"
#include "util.h"
#include "sym.h"

#define HASHSIZE  (NELEM(((sym_tab_t *)0)->bucket))     /* hash bucket size */
#define CHASHSIZE (NELEM(((sym_tab_t *)0)->cbucket))    /* conflict hash bucket size */

#define EQUALP(x) (v.x == p->sym.u.c.v.x)    /* compares value of constant to that of symbol */


/* symbol table */
struct sym_tab_t {
    int scope;              /* scope: constant, global, parameter, local, local+k */
    sym_tab_t *previous;    /* link to table for previous scope */
    struct entry {
        struct sym_t sym;      /* symbol entry */
        struct entry *link;    /* hash chain */
    } *bucket[256];            /* hash buckets */
    sym_t *all;    /* threads all symbols (from latest installed) */
    struct centry {
        const char *name;       /* truncated name */
        sym_t *sym;             /* symbol for truncated name */
        struct centry *link;    /* hash chain */
    } *cbucket[16];             /* hash buckets for conflicts */
};


static sym_tab_t cns = { SYM_SCONST, },     /* table for constants */
                 ext = { SYM_SGLOBAL, },    /* table for externals */
                 ids = { SYM_SGLOBAL, },    /* table for identifiers */
                 tys = { SYM_SGLOBAL, };    /* table for types */


sym_tab_t *sym_const = &cns;     /* constants */
sym_tab_t *sym_extern = &ext;    /* identifiers declared with extern */
sym_tab_t *sym_ident = &ids;     /* ordinary identifiers */
sym_tab_t *sym_global = &ids;    /* identifiers at file scope */
sym_tab_t *sym_type = &tys;      /* type tags */
sym_tab_t *sym_label;            /* generated lables */

int sym_scope = SYM_SGLOBAL;    /* current scope */


static char buf[1+1+18+1+1+4+1];    /* sign+period+prec+sign+e+exp+null; from sym_vtoa() */


/*
 *  creates a symbol table
 */
sym_tab_t *(sym_table)(sym_tab_t *tp, int scope)
{
    sym_tab_t *new;

    new = ARENA_CALLOC(strg_func, 1, sizeof(*new));
    new->previous = tp;
    new->scope = scope;
    if (tp)
        new->all = tp->all;

    return new;
}


/*
 *  invokes a call-back function for each symbol in a symbol table
 */
void (sym_foreach)(sym_tab_t *tp, int scope, void (*apply)(sym_t *, void *), void *cl)
{
    assert(tp);
    assert(apply);

    while (tp && tp->scope > scope)
        tp = tp->previous;
    if (tp && tp->scope == scope) {
        sym_t *p;
        for (p = tp->all; p && p->scope == scope; p = p->up)
            apply(p, cl);
    }
}


/*
 *  enters a scope
 */
void (sym_enterscope)(void)
{
    sym_scope++;
}


/*
 *  exits from the current scope
 */
void (sym_exitscope)(const lmap_t *posm)
{
    int i, n = 0;
    sym_tab_t **tppa[] = { &sym_ident, &sym_type };

    assert(sym_scope >= SYM_SGLOBAL);

    ty_rmtype(sym_scope);
    for (i = 0; i < NELEM(tppa); i++) {
        if ((*tppa[i])->scope == sym_scope) {
            if (main_opt()->std) {
                sym_t *p;
                for (p = (*tppa[i])->all; p && SYM_SAMESCP(p, sym_scope); p = p->up)
                    if (++n > TL_NAMEB_STD) {
                        (void)(err_dpos(lmap_pin(clx_cpos), ERR_PARSE_MANYBID) &&
                               err_dpos(lmap_pin(clx_cpos), ERR_PARSE_MANYBIDSTD,
                                        (long)TL_NAMEB_STD) &&
                               err_dpos(posm, ERR_PARSE_BLOCKSTART));
                        break;
                    }
            }
            *tppa[i] = (*tppa[i])->previous;
        }
    }
    sym_scope--;
}


/*
 *  looks up a name in a table
 */
sym_t *(sym_lookup)(const char *name, sym_tab_t *tp)
{
    unsigned h;
    struct entry *p;

    assert(name);
    assert(tp);

    h = hashkey(name, HASHSIZE);
    do {
        for (p = tp->bucket[h]; p; p = p->link)
            if (name == p->sym.name)
                return &p->sym;
    } while((tp = tp->previous) != NULL);

    return NULL;
}


/*
 *  generates a truncated name for conflict detection
 */
const char *(sym_cname)(const char *name, int glb)
{
    if (main_opt()->std && name && !GENNAME(name)) {
        glb = (glb)? TL_ENAME_STD: TL_INAME_STD;
        if (snlen(name, glb) == glb)
            return hash_new(name, glb);
    }

    return NULL;
}


/*
 *  looks up a truncated name for conflict detection
 */
sym_t *(sym_clookup)(const char *name, sym_tab_t *tp, int glb)
{
    unsigned h;
    struct centry *p;
    const char *cname;

    assert(name);
    assert(tp);

    if ((cname = sym_cname(name, glb)) != NULL) {
        h = hashkey(cname, CHASHSIZE);
        do {
            for (p = tp->cbucket[h]; p; p = p->link)
                if (cname == p->name && name != p->sym->name)
                    return p->sym;
        } while((tp = tp->previous) != NULL);
    }

    return NULL;
}


/*
 *  installs a symbol's truncated name to detect possible conflict
 */
static void cinstall(sym_tab_t *tp, sym_t *sym, arena_t *arena)
{
    unsigned h;
    struct centry *p;
    const char *cname;

    assert(tp);
    assert(sym);

    cname = sym_cname(sym->name, (sym->sclass == LEX_EXTERN ||
                                  (sym->scope == SYM_SGLOBAL && sym->sclass == LEX_AUTO)));
    if (cname) {
        p = ARENA_ALLOC(arena, sizeof(*p));
        p->name = cname;
        p->sym = sym;
        h = hashkey(p->name, CHASHSIZE);
        p->link = tp->cbucket[h];
        tp->cbucket[h] = p;
    }
}


/*
 *  installs a name into a table
 */
static sym_t *install(const char *name, sym_tab_t **tpp, int scls, ty_t *ty, int scope,
                      const lmap_t *pos, arena_t *arena)
{
    unsigned h;
    sym_tab_t *tp;
    struct entry *p;

    assert(name);
    assert(tpp);
    assert(*tpp);
    assert(scope >= (*tpp)->scope);
    assert(pos);

    h = hashkey(name, HASHSIZE);
    tp = *tpp;
    if (scope > tp->scope)
        tp = *tpp = sym_table(tp, scope);
    p = ARENA_CALLOC(arena, 1, sizeof(*p));
    p->sym.name = name;
    p->sym.scope = scope;
    p->sym.type = ty;
    p->sym.sclass = scls;
    p->sym.pos = pos;
    p->sym.up = tp->all;
    tp->all = &p->sym;
    p->link = tp->bucket[h];
    tp->bucket[h] = p;

    cinstall(tp, &p->sym, arena);

    return &p->sym;
}


/*
 *  creates a symbol and installs it if necessary
 */
sym_t *(sym_new)(int kind, ...)
{
    va_list ap;
    sym_t *p, *q;
    const char *name;
    const lmap_t *pos;
    sym_tab_t **tpp;
    int scls = 0;
    ty_t *ty = NULL;
    int scope = sym_scope;
    arena_t *arena = strg_perm;

    assert(kind >= SYM_KORDIN && kind <= SYM_KADDR);

    va_start(ap, kind);

    if (kind <= SYM_KTYPE) {    /* name, pos */
        name = va_arg(ap, const char *);
        pos = va_arg(ap, const lmap_t *);
    }

    switch(kind) {
        case SYM_KORDIN:    /* scls, ty, arena */
            tpp = &sym_ident;
            scls = va_arg(ap, int);
            ty = va_arg(ap, ty_t *);
            /* scope = sym_scope; */
            arena = va_arg(ap, arena_t *);
            break;
        case SYM_KENUM:    /* ty, arena, v */
            tpp = &sym_ident;
            scls = LEX_ENUM;
            ty = va_arg(ap, ty_t *);
            /* scope = sym_scope; */
            arena = va_arg(ap, arena_t *);
            break;
        case SYM_KTYPEDEF:    /* ty, arena */
            tpp = &sym_ident;
            scls = LEX_TYPEDEF;
            ty = va_arg(ap, ty_t *);
            /* scope = sym_scope; */
            arena = va_arg(ap, arena_t *);
            break;
        case SYM_KGLOBAL:    /* scls, ty */
            tpp = &sym_global;
            scls = va_arg(ap, int);
            ty = va_arg(ap, ty_t *);
            scope = SYM_SGLOBAL;
            /* arena = strg_perm; */
            break;
        case SYM_KEXTERN:    /* scls, ty */
            tpp = &sym_extern;
            scls = va_arg(ap, int);
            ty = va_arg(ap, ty_t *);
            scope = SYM_SGLOBAL;
            /* arena = strg_perm; */
            break;
        case SYM_KLABEL:    /* tpp */
            tpp = va_arg(ap, sym_tab_t **);
            /* scls = 0; */
            /* ty = NULL; */
            scope = SYM_SLABEL;
            arena = strg_func;
            break;
        case SYM_KTAG:
            tpp = &sym_type;
            /* scls = 0; */
            /* ty = NULL; */
            scope = sym_scope;
            /* arena = strg_perm; */
            break;
        case SYM_KTYPE:    /* outofline */
            tpp = &sym_type;
            /* scls = 0; */
            /* ty = NULL; */
            scope = SYM_SGLOBAL;
            /* arena = strg_perm; */
            break;
        /* following symbols generated within switch */
        case SYM_KTEMPB:    /* scls, op code */
            scls = va_arg(ap, int);
            ty = op_stot(va_arg(ap, int));
            scope = SYM_SLOCAL;
            goto symgen;
        case SYM_KADDR:    /* symbol, ty */
            q = va_arg(ap, sym_t *);
            ty = va_arg(ap, ty_t *);
            scls = q->sclass;
            scope = q->scope;
            goto symgen;
        case SYM_KTEMP:    /* scls, ty, scope */
        case SYM_KGEN:
            scls = va_arg(ap, int);
            ty = va_arg(ap, ty_t *);
            scope = va_arg(ap, int);
        symgen:
            arena = (scope < SYM_SPARAM)? strg_perm: strg_func;
            assert(ir_cur);
            assert(ty && ((ty->op >= TY_FLOAT && ty->op <= TY_ARRAY) || ty->op & TY_CONSVOL));
            assert(kind != SYM_KTEMP || (scls == LEX_AUTO || scls == LEX_REGISTER));
            p = ARENA_CALLOC(arena, 1, sizeof(*p));
            p->name = hash_int(sym_genlab(1));
            p->scope = scope;
            p->sclass = scls;
            p->type = ty;
            if (kind != SYM_KADDR && scope == SYM_SGLOBAL)
                ir_cur->symgsc(p);
            else
                switch(kind) {
                    case SYM_KTEMPB:
                        ir_cur->symlocal(p);
                        p->f.defined = 1;
                        /* no break */
                    case SYM_KTEMP:
                        p->f.temporary = 1;
                        break;
                    case SYM_KADDR:
                        p->f = q->f;
                        p->f.computed = 1;
                        p->f.defined = 1;
                        break;
                }
            return p;
        default:
            assert(!"invalid symbol kind -- should never reach here");
            break;
    }

    assert(name);
    assert(tpp && (*tpp)->scope >= SYM_SCONST);
    assert((scls == 0 && (kind == SYM_KLABEL || kind == SYM_KTAG || kind == SYM_KTYPE)) ||
           (scls == LEX_ENUM && kind == SYM_KENUM) ||
           (scls >= LEX_AUTO && scls <= LEX_TYPEDEF));
    assert((ty == NULL && (kind == SYM_KLABEL || kind == SYM_KTAG || kind == SYM_KTYPE)) ||
           (ty && ((ty->op >= TY_FLOAT && ty->op <= TY_ARRAY) || ty->op & TY_CONSVOL ||
                   ty->op == TY_UNKNOWN)));
    assert(scope >= SYM_SCONST);
    assert(arena == strg_perm || arena == strg_func || arena == strg_stmt);

    p = install(name, tpp, scls, ty, scope, pos, arena);

    switch(kind) {
        case SYM_KENUM:
            p->u.value = va_arg(ap, sx_t);    /* v */
            break;
        case SYM_KLABEL:
            p->u.l.label = sym_genlab(1);
            break;
        case SYM_KTYPE:
            p->f.outofline = va_arg(ap, int);    /* outofline */
            break;
    }

    va_end(ap);
    return p;
}


/*
 *  finds a label in a symbol table; creates one if not found
 */
sym_t *(sym_findlabel)(int lab)
{
    struct entry *p;
    unsigned h = lab & (HASHSIZE-1);

    assert(ir_cur);

    for (p = sym_label->bucket[h]; p; p = p->link)
        if (lab == p->sym.u.l.label)
            return &p->sym;
    p = ARENA_CALLOC(strg_func, 1, sizeof(*p));
    p->sym.name = hash_int(lab);
    p->sym.scope = SYM_SLABEL;
    p->sym.up = sym_label->all;
    sym_label->all = &p->sym;
    p->link = sym_label->bucket[h];
    sym_label->bucket[h] = p;
    p->sym.u.l.label = lab;
    ir_cur->symgsc(&p->sym);

    return &p->sym;
}


/*
 *  finds a constant in a symbol table; creates one if not found
 *  ASSUMPTION: unsigned integers are compatible with signed ones on the host
 */
sym_t *(sym_findconst)(ty_t *ty, sym_val_t v)
{
    struct entry *p;
    unsigned h = (unsigned)v.u & (HASHSIZE-1);

    assert(ty);
    assert(ir_cur);

    ty = TY_UNQUAL(ty);    /* constants are rvalues */
    for (p = sym_const->bucket[h]; p; p = p->link)
        if (ty_equiv(ty, p->sym.type, 1))
            switch(ty->op) {
                case TY_CHAR:
                case TY_SHORT:
                case TY_INT:
                case TY_ENUM:        /* no need for TY_RMQENUM() */
                case TY_UNSIGNED:
                case TY_LONG:
                case TY_ULONG:
                    if (EQUALP(u))
                        return &p->sym;
                    break;
                case TY_FLOAT:
                    if (EQUALP(f))
                        return &p->sym;
                    break;
                case TY_DOUBLE:
                    if (EQUALP(d))
                        return &p->sym;
                    break;
                case TY_LDOUBLE:
                    if (EQUALP(ld))
                        return &p->sym;
                    break;
                case TY_ARRAY:
                case TY_FUNCTION:
                case TY_POINTER:
                    if (EQUALP(tp))
                        return &p->sym;
                    break;
                default:
                    assert(!"invalid type operator -- should never reach here");
                    break;
            }

    p = ARENA_CALLOC(strg_perm, 1, sizeof(*p));
    p->sym.name = sym_vtoa(ty, v);
    p->sym.scope = SYM_SCONST;
    p->sym.type = ty;
    p->sym.sclass = LEX_STATIC;
    p->sym.u.c.v = v;
    p->link = sym_const->bucket[h];
    p->sym.up = sym_const->all;
    sym_const->all = &p->sym;
    sym_const->bucket[h] = p;
    if (ty->u.sym && !ty->u.sym->f.outofline)
        ir_cur->symgsc(&p->sym);
    p->sym.f.defined = 1;

    return &p->sym;
}


/*
 *  finds an integer constant in a symbol table; creates one if not found
 */
sym_t *(sym_findint)(long n)
{
    sym_val_t v;

    assert(ty_longtype);    /* ensures types initialized */

    v.s = n;
    return sym_findconst(ty_longtype, v);
}


/*
 *  generates a label; also used for other identifiers
 */
int (sym_genlab)(int n)
{
    static int label = 1;

    assert(n > 0);

    label += n;
    return label - n;
}


/*
 *  makes up a semi-generated identifier;
 *  semi-generated ids are not detected by GENNAME() but never printed out;
 *  see also symstr() from err.c
 */
const char *(sym_semigenlab)(void)
{
    assert(sizeof(buf) >= 1 + BUFN + 1);

    sprintf(buf, "#%d", sym_genlab(1));
    return hash_string(buf);
}


/*
 *  adds use information to a symbol
 */
void (sym_use)(sym_t *p, const lmap_t *pos)
{
    assert(p);
    assert(pos);

    p->use = alist_append(p->use, (void *)pos, strg_perm);
}


/*
 *  constructs a type list entry
 */
sym_tylist_t *(sym_tylist)(sym_tylist_t *p, sym_t *sym)
{
    sym_tylist_t *q = ARENA_ALLOC(strg_perm, sizeof(*q));

    assert(sym);

    q->next = p;
    q->type = sym->type;
    q->pos = sym->pos;

    return q;
}


/*
 *  sign-extends integers to store into bit-fields;
 *  ASSUMPTION: 2sC for signed integers assumed;
 *  ASSUMPTION: bit-field is signed or unsigned int;
 *  ASSUMPTION: bitwise operations on signed types work as expected
 */
sx_t (sym_sextend)(sx_t v, sym_field_t *p)
{
    assert(p);
    assert(SYM_FLDSIZE(p) < TG_CHAR_BIT*p->type->size);

    v &= SYM_FLDMASK(p);
    if (v >> (SYM_FLDSIZE(p)-1))
        v |= (~0UL ^ SYM_FLDMASK(p));

    return v;
}


/*
 *  converts a constant to string representation;
 *  integer representations used by the back-end (see symgsc());
 *  ASSUMPTION: unsigned long can represent void * on the host
 */
const char *(sym_vtoa)(const ty_t *ty, sym_val_t v)
{
    assert(ty);
    assert(sizeof(buf) >= 1 + BUFN + 1);
    assert(sizeof(buf) >= 2 + (sizeof(ux_t)*CHAR_BIT+3)/4 + 1);

    ty = TY_RMQENUM(ty);
    switch(ty->op) {
        case TY_CHAR:
        case TY_SHORT:
        case TY_INT:
        case TY_LONG:
            sprintf(buf, "%"FMTMX"d", v.s);
            break;
        case TY_UNSIGNED:
        case TY_ULONG:
            sprintf(buf, "%"FMTMX"u", v.u);
            break;
        case TY_FLOAT:
            sprintf(buf, "%.8g", v.f);
            break;
        case TY_DOUBLE:
            sprintf(buf, "%.18g", v.d);
            break;
        case TY_LDOUBLE:
            sprintf(buf, "%.18Lg", v.ld);
            break;
        case TY_ARRAY:
            if (ty->type->op == TY_CHAR)    /* array of char */
                return v.hp;
        case TY_FUNCTION:
        case TY_POINTER:
            sprintf(buf, "0x%lx", (unsigned long)v.tp);
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            return NULL;
    }

    return hash_string(buf);
}

/* end of sym.c */
