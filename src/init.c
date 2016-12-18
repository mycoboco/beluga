/*
 *  initializer parsing
 */

#include <stddef.h>        /* NULL */
#include <cbl/assert.h>    /* assert */

#include "clx.h"
#include "common.h"
#include "enode.h"
#include "err.h"
#include "expr.h"
#include "ir.h"
#include "lex.h"
#include "op.h"
#include "sset.h"
#include "simp.h"
#include "sym.h"
#include "tree.h"
#include "ty.h"
#include "init.h"

/* addition and ROUNDUP() with overflow check */
#define ADD(n, s)      (((n) > LONG_MAX-(s))? (ovf=1, n): (n)+(s))
#define RNDUPOVF(x, n) (((x) > LONG_MAX-((n)-1))? (ovf=1, x): ((x)+((n)-1)) & (~((n)-1)))


static int ovf;       /* overflow occurred in type size */
static int curseg;    /* current segment */


/*
 *  accepts extra braces for initializers
 */
static int extrabrace(int lev)
{
    if (lev < 0) {
        lev = 0;
        if (clx_tc == '{') {
            do {
                lev++;
                if (lev == 2)
                    err_dpos(clx_cpos, ERR_PARSE_EXTRABRACE);
                clx_tc = clx_next();
            } while(clx_tc == '{');
        }
    } else {
        while (lev-- > 0) {
            if (clx_tc == ',')
                clx_tc = clx_next();
            clx_xtracomma('}', "initializer", 1);
            sset_test('}', (sym_scope >= SYM_SPARAM)? sset_initb: sset_initf);
        }
    }

    return lev;
}


/*
 *  checks and generates a constant expression
 */
static long genconst(tree_t **pe)
{
    tree_t *e;

    assert(pe);
    assert(ir_cur);

    if (!*pe)
        return -1;
    else
        e = *pe;

    switch(op_generic(e->op)) {
        case OP_ADDRG:
            if (e->f.npce & (TREE_FCOMMA|TREE_FADDR))
                err_dpos(e->pos, ERR_EXPR_INVINITCE);
            ir_cur->initaddr(e->u.sym);
            return e->type->size;
        case OP_CNST:
            assert(!TY_ISARRAY(e->type));
            if (e->f.npce & ((TY_ISPTR(e->type))? (TREE_FCOMMA|TREE_FADDR):
                                                  (TREE_FCOMMA|TREE_FACE)))
                err_dpos(e->pos, ERR_EXPR_INVINITCE);
            ir_cur->initconst(op_sfx(e->type), e->u.v);
            return e->type->size;
        default:
            err_dpos(e->pos, ERR_PARSE_INITCONST);
            *pe = NULL;
            return -1;
    }

    /* assert(!"impossible control flow -- should never reach here");
       return e->type->size; */
}


/*
 *  parses initializers for integer types
 */
static tree_t *intinit(ty_t *ty)
{
    int b;
    ty_t *aty;
    tree_t *e;

    assert(ty);
    assert(!TY_ISQUAL(ty));
    assert(TY_ISINTEGER(ty));

    simp_needconst++;

    b = extrabrace(-1);
    e = expr_asgn(0, 0, 1);
    extrabrace(b);
    if (!e)
        goto ret;
    if ((aty = enode_tcasgnty(ty, e, e->pos)) != NULL) {    /* need not check for ty_voidtype */
        e = enode_cast(e, aty, ENODE_FCHKOVF, e->pos);
        if (op_generic(e->op) != OP_CNST) {
            err_dpos(e->pos, ERR_PARSE_INITCONST);
            e = NULL;
        } else if (e->f.npce & (TREE_FCOMMA|TREE_FACE))
            err_dpos(e->pos, ERR_EXPR_INVINITCE);
    } else {
        err_dpos(e->pos, ERR_PARSE_INVINIT, e->type, ty);
        e = NULL;
    }

    ret:
        simp_needconst--;
        return e;
}


/*
 *  parses initializers for a non-char array;
 *  ASSUMPTION: size of array that is too big should be adjusted
 */
static long arrayinit(int stop, ty_t *ty, int lev, const lmap_t *pos)
{
    ty_t *aty;
    long n = 0;
    int issue = 0;

    assert(ty);
    assert(ty->type);
    assert(!TY_ISQUAL(ty));
    assert(stop != 1 || ty->size > 0);

    aty = TY_UNQUAL(ty->type);
    assert(aty);

    while (1) {
        init_init(aty, lev, pos);
        n = ADD(n, aty->size);
        if (stop == 2 || (stop && (n >= ty->size || ovf)) || clx_tc != ',')
            break;
        clx_tc = clx_next();
        clx_xtracomma('}', "initializer for array", 1);
        if (clx_tc == '}')
            break;
        if (!issue && ty->size > 0 && (n >= ty->size || ovf)) {
            err_dpos(clx_cpos, ERR_PARSE_MANYINIT, ty);
            issue = 1;
        }
    }

    return n;
}


/*
 *  parses initalizers for a char array
 */
static long carrayinit(int stop, ty_t *ty)
{
    long n = 0;
    int issue = 0;
    char buf[sizeof(long)], *s = buf;

    assert(ty);
    assert(ty->type);
    assert(!TY_ISQUAL(ty));
    assert(stop != 1 || ty->size > 0);
    assert(ty_uchartype);    /* ensures types initialized */
    assert(ir_cur);
    assert(sizeof(long) >= ty_inttype->size);

    while (1) {
        tree_t *t = intinit(TY_UNQUAL(ty->type));
        *s++ = SYM_CROPUC((t)? t->u.v.s: 0);
        n = ADD(n, 1);
        if (n % ty_inttype->size == 0) {
            ir_cur->initstr(ty_inttype->size, buf);
            s = buf;
        }
        if (stop == 2 || (stop && (n >= ty->size || ovf)) || clx_tc != ',')
            break;
        clx_tc = clx_next();
        clx_xtracomma('}', "initializer for array", 1);
        if (clx_tc == '}')
            break;
        if (!issue && ty->size > 0 && (n >= ty->size || ovf)) {
            err_dpos(clx_cpos, ERR_PARSE_MANYINIT, ty);
            issue = 1;
        }
    }
    if (s > buf)
        ir_cur->initstr(s - buf, buf);

    return n;
}


/*
 *  skips initializers:
 *
 *           +-{-+ <----!}----+
 *            \ /             |
 *      s -{-> 1 -!{-> 2 -,-> 3 -}-> 4 -lev=0||other-> e
 *                     |        <-,-/ \
 *                     +----}----> +-}-+
 *                     |
 *                     +------------other------------> e
 *
 *  where the number of closing braces checked at e
 */
void (init_skip)(void)
{
    int lev = 0;

    if (clx_tc == '{') {
        while (1) {
            do {    /* 1 */
                while (clx_tc == '{') {
                    lev++;
                    clx_tc = clx_next();
                }
                expr_asgn(0, 0, 0);    /* 2 */
                if (clx_tc != ',')
                    break;
                clx_tc = clx_next();    /* 3 */
                clx_xtracomma('}', "initializer", 1);
            } while(clx_tc != '}');
            do {    /* 4 */
                while (clx_tc == '}' && lev > 0) {
                    lev--;
                    clx_tc = clx_next();
                }
                if (lev == 0 || clx_tc != ',')
                    goto e;
                clx_tc = clx_next();
                clx_xtracomma('}', "initializer", 1);
            } while(clx_tc == '}');
        }
        e:
            while (lev-- > 0)
                sset_expect('}');
    } else
        expr_asgn(0, 0, 0);
}


/*
 *  parses initializers for bit-fields in a storage unit;
 *  ASSUMPTION: bit-field is signed or unsigned int;
 *  ASSUMPTION: overflow of left shift is silently ignored on the host;
 *  ASSUMPTION: 2sC for signed integers assumed
 */
static long fieldinit(sym_field_t *p, sym_field_t *q)
{
    tree_t *e;
    int i, n = 0;
    unsigned long ul = 0;

    assert(p);
    assert(ty_longtype);    /* ensures types initialized */
    assert(ir_cur);

    do {
        if (TY_ISINT(p->type)) {
            long li;
            e = intinit(ty_longtype);
            li = (e)? e->u.v.s: 0;
            if (SYM_FLDSIZE(p) < TG_CHAR_BIT*p->type->size) {
                if (!SYM_INFIELD(li, p))
                    err_dpos(e->pos, ERR_PARSE_BIGFLDINIT);
                li &= SYM_FLDMASK(p);
            }
            ul |= SYM_CROPUI(li << SYM_FLDRIGHT(p));
        } else {
            unsigned long u;
            assert(TY_ISUNSIGNED(p->type));
            e = intinit(ty_ulongtype);
            u = (e)? e->u.v.u: 0;
            if (SYM_FLDSIZE(p) < TG_CHAR_BIT*p->type->size)
                u &= SYM_FLDMASK(p);
            ul |= SYM_CROPUI(u << SYM_FLDRIGHT(p));
        }
        if (ir_cur->f.little_bit) {
            if (SYM_FLDSIZE(p) + SYM_FLDRIGHT(p) > n)
                n = SYM_FLDSIZE(p) + SYM_FLDRIGHT(p);
        } else {
            if (SYM_FLDSIZE(p) + SYM_FLDLEFT(p) > n)
                n = SYM_FLDSIZE(p) + SYM_FLDLEFT(p);
        }
        if (p->link == q)
            break;
        p = p->link;
    } while(clx_tc == ',' && (clx_tc = clx_next()) != LEX_EOI &&
            !clx_xtracomma('}', "initializer for bit-field", 1) && clx_tc != '}');

    n = (n + TG_CHAR_BIT - 1) / TG_CHAR_BIT;
    for (i = 0; i < n; i++) {
        sym_val_t v;
        if (ir_cur->f.little_endian) {
            v.u = SYM_CROPUC(ul);
            ul >>= TG_CHAR_BIT;
        } else {
            v.u = SYM_CROPUC(ul >> (TG_CHAR_BIT*(ty_unsignedtype->size - 1)));
            ul <<= TG_CHAR_BIT;
        }
        ir_cur->initconst(op_sfx(ty_chartype), v);
    }

    return n;
}


/*
 *  parses initializers for structs
 */
static long structinit(int stop, ty_t *ty, int lev, const lmap_t *pos)
{
    int a;
    long n = 0;
    int issue = 0;
    sym_field_t *p;

    assert(ty);
    assert(TY_ISSTRUNI(ty));
    assert(!stop || ty->size > 0);
    assert(ir_cur);

    p = TY_UNQUAL(ty)->u.sym->u.s.flist;
    assert(p);    /* incomplete struct excluded */

    do {
        if (!p) {
            if (!issue) {
                err_dpos(clx_cpos, ERR_PARSE_MANYINIT, ty);
                issue = 1;
            }
            init_skip();
        } else {
            if (p->offset > n) {
                ir_cur->initspace(p->offset - n);
                n = ADD(n, p->offset - n);
            }
            if (p->lsb) {
                long m;
                sym_field_t *q = p;
                if (TY_ISSTRUCT(ty))
                    while (q->link && q->link->offset == p->offset)
                        q = q->link;
                m = fieldinit(p, q->link);
                n = ADD(n, m);
                p = q;
            } else {
                init_init(p->type, lev, pos);
                n = ADD(n, p->type->size);
            }
            p = p->link;
            a = (p)? p->type->align: ty->align;
            if (a != 0 && n % a != 0) {
                ir_cur->initspace(a - n%a);
                n = RNDUPOVF(n, a);
            }
        }
        if (TY_ISUNION(ty))
            p = NULL;
        if (stop == 2 || (stop && !p) || clx_tc != ',')
            break;
        clx_tc = clx_next();
        clx_xtracomma('}', "initializer for member", 1);
    } while(clx_tc != '}');

    return n;
}


/*
 *  parses initializers;
 *  TODO: initializing incomplete types in a function issues two messages
 */
ty_t *(init_init)(ty_t *ty, int lev, const lmap_t *pos)
{
    int b;
    long n = 0;
    tree_t *e;
    ty_t *aty = NULL;

    assert(ty);
    assert(ty_wchartype);    /* ensures types initialized */
    assert(ir_cur);

    ovf = 0;
    if (TY_ISUNKNOWN(ty)) {
        init_skip();
        return ty;
    } else if (TY_ISSCALAR(ty)) {
        simp_needconst++;
        b = extrabrace(-1);
        e = expr_asgn(0, 0, 1);
        extrabrace(b);
        if (e) {
            e = enode_value(enode_pointer(e, e->pos), e->pos);
            if ((aty = enode_tcasgnty(ty, e, e->pos)) != NULL)    /* no check for ty_voidtype */
                e = enode_cast(e, aty, ENODE_FCHKOVF, e->pos);
            else {
                err_dpos(e->pos, ERR_PARSE_INVINIT, e->type, ty);
                e = NULL;
            }
            n = genconst(&e);
        }
        if (!e)
            n = ty->size;
        simp_needconst--;
    } else if (TY_ISSTRUNI(ty)) {
        if (ty->size == 0) {
            err_dpos(pos, ERR_PARSE_INCOMINIT, ty);
            init_skip();
            return ty;
        }
        if (TY_ISUNION(ty)) {
            if (clx_tc == '{') {
                clx_tc = clx_next();
                n = structinit(0, ty, lev + 1, pos);
                extrabrace(1);
            } else {
                if (lev == 0)
                   err_dpos(clx_cpos, ERR_PARSE_NOBRACE, ty);
                n = structinit(2, ty, lev + 1, pos);
            }
        } else {
            assert(TY_ISSTRUCT(ty));
            if (clx_tc == '{') {
                clx_tc = clx_next();
                n = structinit(0, ty, lev + 1, pos);
                extrabrace(1);
            } else if (lev > 0)
                n = structinit(1, ty, lev + 1, pos);
            else {
                err_dpos(clx_cpos, ERR_PARSE_NOBRACE, ty);
                n = structinit(2, ty, lev + 1, pos);
            }
        }
    } else if (TY_ISARRAY(ty)) {
        aty = TY_UNQUAL(ty->type);
        b = (TY_ISCHAR(aty))? 1: (aty->t.type == ty_wchartype)? 2: 0;
        if (b && clx_tc == LEX_SCON) {
            if (b == 1 && clx_sym->type->type == ty_wchartype)
                err_dpos(pos, ERR_PARSE_INVMBINIT);
            else if (b == 2 && TY_ISCHAR(clx_sym->type->type))
                err_dpos(pos, ERR_PARSE_INVWINIT);
            else if (ty->size > 0 && ty->size == clx_sym->type->size - aty->size)
                clx_sym->type = ty_array(aty, ty->size/aty->size, clx_cpos);
            n = clx_sym->type->size;
            ir_cur->initstr(clx_sym->type->size, clx_sym->u.c.v.hp);
            if (ty->size > 0 && n > ty->size)
                err_dpos(clx_cpos, ERR_PARSE_MANYINIT, ty);
            clx_tc = clx_next();
        } else if (clx_tc == '{') {
            clx_tc = clx_next();
            if (b && clx_tc == LEX_SCON) {
                ty = init_init(ty, lev + 1, pos);
                extrabrace(1);
                return ty;
            }
            n = (b == 1)? carrayinit(0, ty): arrayinit(0, ty, lev + 1, pos);
            extrabrace(1);
        } else if (lev > 0 && ty->size > 0) {
            n = (b == 1)? carrayinit(1, ty): arrayinit(1, ty, lev + 1, pos);
        } else {
            err_dpos(clx_cpos, ERR_PARSE_NOBRACE, ty);
            n = (b == 1)? carrayinit(2, ty): arrayinit(2, ty, lev + 1, pos);
        }
    } else {
        init_skip();
        return ty;
    }
    if (ty->size) {
        if (n < ty->size)
            ir_cur->initspace(ty->size - n);
    } else if (TY_ISARRAY(ty) && ty->type->size > 0)
        ty = ty_array(ty->type, (ovf)? -1: n / ty->type->size, pos);
    else
        ty->size = n;

    return ty;
}


/*
 *  switches the current segment to another if necessary
 */
int (init_swtoseg)(int seg)
{
    int n = curseg;

    assert(ir_cur);

    if (n != seg)
        ir_cur->segment(seg);
    curseg = seg;

    return n;
}


/*
 *  returns the current segment
 */
int (init_curseg)(void)
{
    return curseg;
}

/* end of init.c */
