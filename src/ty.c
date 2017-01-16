/*
 *  type system
 */

#include <float.h>         /* FLT_MAX, FLT_MIN, DBL_MAX, DBL_MIN, LDBL_MAX, LDBL_MIN */
#include <stdarg.h>        /* va_list, va_start, va_arg, va_end */
#include <stddef.h>        /* NULL */
#include <stdio.h>         /* sprintf */
#include <string.h>        /* strcpy */
#include <cbl/arena.h>     /* ARENA_CALLOC, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_string, hash_int */

#include "alist.h"
#include "clx.h"
#include "common.h"
#include "decl.h"
#include "err.h"
#include "ir.h"
#include "lex.h"
#include "main.h"
#include "strg.h"
#include "sym.h"
#include "ty.h"


/* checks if two given types are enum and compatible integer;
   t1 and t2 are assumed to be not qualified */
#define CMPENUM(t1, t2) ((TY_ISENUM(t1) && (t1)->type == (t2)->t.type) ||    \
                         (TY_ISENUM(t2) && (t2)->type == (t1)->t.type))

#define T(p) ((ty_t *)(p))    /* shorthand for cast to ty_t * */


/* built-in types + pointer to void */
ty_t *ty_unknowntype;     /* unknown type */
ty_t *ty_chartype;        /* plain char */
ty_t *ty_schartype;       /* signed char */
ty_t *ty_uchartype;       /* unsigned char */
ty_t *ty_shorttype;       /* short */
ty_t *ty_ushorttype;      /* unsigned short */
ty_t *ty_inttype;         /* int */
ty_t *ty_unsignedtype;    /* unsigned */
ty_t *ty_longtype;        /* long */
ty_t *ty_ulongtype;       /* unsigned long */
ty_t *ty_floattype;       /* float */
ty_t *ty_doubletype;      /* double */
ty_t *ty_ldoubletype;     /* long double */
ty_t *ty_voidtype;        /* void */
ty_t *ty_voidptype;       /* pointer to void */

/* frequently used types; point to built-in types */
ty_t *ty_sizetype;       /* size_t */
ty_t *ty_ptrdifftype;    /* ptrdiff_t */
ty_t *ty_ptruinttype;    /* unsigned integer type for pointer arithmetic */
ty_t *ty_ptrsinttype;    /* signed integer type for pointer arithmetic */
ty_t *ty_wuchartype;     /* unsigned counterpart of wchar_t */
ty_t *ty_wchartype;      /* wchar_t */
ty_t *ty_winttype;       /* wint_t */


/* type table */
static struct entry {
    struct ty_t type;      /* type entry */
    struct entry *link;    /* hash chain */
} *typetable[128];

static int maxlevel;         /* max scope level of type stored in type table */
static sym_t *pointersym;    /* symbol entry for all pointer types */

/* buffer for numerical values; used in ty_outtype() and ty_outdecl() */
static char num[2 + 1 + BUFN + 1];


/*
 *  creates a basic type or derives a type from another type
 */
static ty_t *type(int op, ty_t *ty, long size, int align, void *sym)
{
    unsigned h = (op^((unsigned)ty>>3)) & (NELEM(typetable)-1);
    struct entry *tn;

    /* functions and incomplete arrays always make new entries */
    if (op != TY_FUNCTION && !(op == TY_ARRAY && size == 0))
        for (tn = typetable[h]; tn; tn = tn->link)
            if (tn->type.op == op && tn->type.type == ty && tn->type.size == size &&
                tn->type.align == align && tn->type.u.sym == sym)
                return &tn->type;
    tn = ARENA_CALLOC(strg_perm, 1, sizeof(*tn));
    tn->type.op = op;
    tn->type.type = ty;
    tn->type.size = size;
    tn->type.align = align;
    tn->type.u.sym = sym;
    tn->type.t.type = &tn->type;
    if (ty)
        tn->type.t.unknown = ty->t.unknown;
    tn->link = typetable[h];
    typetable[h] = tn;

    return &tn->type;
}


/*
 *  assigns max/min values to primitive types;
 *  ASSUMPTION: 2sC for signed integers assumed;
 *  ASSUMPTION: fp types of the host are same as those of the target
 */
static void setmaxmin(ty_t *ty)
{
    int sign = 0;

    assert(ty);

    switch(ty->op) {
        case TY_CHAR:    /* plain/signed/unsigned char */
            if (TY_ISSCHAR(ty))
                sign = 1;
            break;
        case TY_SHORT:    /* signed/unsigned short */
            if (ty == ty_shorttype)
                sign = 1;
            break;
        case TY_INT:
        case TY_LONG:
            sign = 1;
            break;
        case TY_FLOAT:
            ty->u.sym->u.lim.max.f = FLT_MAX;
            ty->u.sym->u.lim.min.f = FLT_MIN;
            return;
        case TY_DOUBLE:
            ty->u.sym->u.lim.max.d = DBL_MAX;
            ty->u.sym->u.lim.min.d = DBL_MIN;
            return;
        case TY_LDOUBLE:
            ty->u.sym->u.lim.max.ld = LDBL_MAX;
            ty->u.sym->u.lim.min.ld = LDBL_MIN;
            return;
    }

    if (sign) {
        ty->u.sym->u.lim.max.s = ONES(ty->size*TG_CHAR_BIT - 1);
        ty->u.sym->u.lim.min.s = -ty->u.sym->u.lim.max.s - 1;
    } else {
        ty->u.sym->u.lim.max.u = ONES(ty->size*TG_CHAR_BIT);
        ty->u.sym->u.lim.min.u = 0;
    }
}


#define INIT(v, name, op, metric)                                                    \
            do {                                                                     \
                sym_t *p = sym_new(SYM_KTYPE, hash_string(name), &dummy,             \
                                   ir_cur->metric.outofline);                        \
                v = type(op, NULL, ir_cur->metric.size, ir_cur->metric.align, p);    \
                p->type = v;    /* has circular reference */                         \
                assert(v->align == 0 || v->size % v->align == 0);                    \
                setmaxmin(v);                                                        \
            } while(0)

/*
 *  initializes built-in types;
 *  ASSUMPTION: the size of type is a multiple of its alignment factor
 *  ASSUMPTION: size_t is either unsigned or unsigned long;
 *  ASSUMPTION: ptrdiff_t is either int or long;
 *  ASSUMPTION: there is an integer type that holds void * without loss of information;
 *  ASSUMPTION: ptruint is either unsigned or unsigned long and has the same size as void *;
 *  ASSUMPTION: ptrsint is either int or long and has the same size as void *
 */
void (ty_init)(void)
{
    static const lmap_t dummy;

    assert(!ty_chartype);
    assert(ir_cur);

    INIT(ty_unknowntype,  "unknown type",   TY_UNKNOWN,  charmetric);
    ty_unknowntype->t.unknown = 1;
    INIT(ty_chartype,     "char",           TY_CHAR,     charmetric);
    INIT(ty_schartype,    "signed char",    TY_CHAR,     charmetric);
    INIT(ty_uchartype,    "unsigned char",  TY_CHAR,     charmetric);
    INIT(ty_shorttype,    "short",          TY_SHORT,    shortmetric);
    INIT(ty_ushorttype,   "unsigned short", TY_SHORT,    shortmetric);
    INIT(ty_inttype,      "int",            TY_INT,      intmetric);
    INIT(ty_unsignedtype, "unsigned int",   TY_UNSIGNED, intmetric);
    INIT(ty_longtype,     "long int",       TY_LONG,     longmetric);
    INIT(ty_ulongtype,    "unsigned long",  TY_ULONG,    longmetric);
    INIT(ty_floattype,    "float",          TY_FLOAT,    floatmetric);
    INIT(ty_doubletype,   "double",         TY_DOUBLE,   doublemetric);
    INIT(ty_ldoubletype,  "long double",    TY_LDOUBLE,  ldoublemetric);
    {
        sym_t *p = sym_new(SYM_KTYPE, hash_string("void"), &dummy, 0);
        ty_voidtype = type(TY_VOID, NULL, 0, 0, p);
        p->type = ty_voidtype;    /* has circular reference */
    }
    pointersym = sym_new(SYM_KTYPE, hash_string("T*"), &dummy, ir_cur->ptrmetric.outofline);
    ty_voidptype = ty_ptr(ty_voidtype);
    pointersym->type = ty_voidptype;    /* ty_ptr() requires pointersym initialized */
    assert(ty_voidptype->align > 0 && ty_voidptype->size % ty_voidptype->align == 0);

    ty_sizetype = (main_opt()->sizet)? ty_ulongtype: ty_unsignedtype;
    ty_ptrdifftype = (main_opt()->ptrdifft)? ty_longtype: ty_inttype;
    if (main_opt()->ptrlong) {
        ty_ptruinttype = ty_ulongtype;
        ty_ptrsinttype = ty_longtype;
    } else {
        ty_ptruinttype = ty_unsignedtype;
        ty_ptrsinttype = ty_inttype;
    }
    assert(ty_ptruinttype->size == ty_voidptype->size);
    assert(ty_ptrsinttype->size == ty_voidptype->size);
    switch(main_opt()->wchart) {
        case 0:    /* wchar_t = long */
            ty_wuchartype = ty_ulongtype;
            ty_wchartype = ty_longtype;
            ty_winttype = ty_longtype;
            break;
        case 1:    /* wchar_t = u-short */
            ty_wuchartype = ty_ushorttype;
            ty_wchartype = ty_ushorttype;
            ty_winttype = ty_inttype;
            break;
        case 2:    /* wchar_t = int */
            ty_wuchartype = ty_unsignedtype;
            ty_wchartype = ty_inttype;
            ty_winttype = ty_inttype;
            break;
        default:
            assert(!"invalid option parameter -- should never reach here");
            break;
    }
}

#undef INIT


/*
 *  removes types from the type table when exiting a scope
 */
void (ty_rmtype)(int lev)
{
    if (maxlevel >= lev) {
        int i;
        maxlevel = 0;
        for (i = 0; i < NELEM(typetable); i++) {
            struct entry *tn, **tq = &typetable[i];
            while ((tn = *tq) != NULL)
                if (tn->type.op == TY_FUNCTION)
                    tq = &tn->link;
                else if (tn->type.u.sym && tn->type.u.sym->scope >= lev)
                    *tq = tn->link;
                else {
                    if (tn->type.u.sym && tn->type.u.sym->scope > maxlevel)
                        maxlevel = tn->type.u.sym->scope;
                    tq = &tn->link;
                }
        }
    }
}


/*
 *  generates a pointer to ty;
 *  ASSUMPTION: all pointers are uniform (same size and same alignment factor)
 */
ty_t *(ty_ptr)(ty_t *ty)
{
    assert(ty);
    assert(pointersym);    /* ensures types initialized */
    assert(ir_cur);

    return type(TY_POINTER, ty, ir_cur->ptrmetric.size, ir_cur->ptrmetric.align, pointersym);
}


/*
 *  dereferences a pointer;
 *  if ty is not a pointer, ty returned
 */
ty_t *(ty_deref)(ty_t *ty)
{
    assert(ty);

    return (TY_ISPTR(ty))? TY_UNQUAL(ty)->type: NULL;
}


/*
 *  constructs an array;
 *  if ty is a function, an array of function pointers returned;
 *  an array of an incomplete type might be returned;
 *  if the size is too long, adjusted to 1
 */
ty_t *(ty_array)(ty_t *ty, long n, const lmap_t *pos)
{
    assert(ty);
    assert(ty_longtype);    /* ensures types initialized */

    if (TY_ISFUNC(ty)) {
        err_dpos(pos, ERR_TYPE_ARRFUNC);
        return ty_array(ty_ptr(ty), n, pos);    /* returns array of function pointers */
    }
    if (TY_ISVOID(ty))    /* array of void */
        err_dpos(pos, ERR_TYPE_ARRVOID);
    else if (TY_ISARRAY(ty) && ty->size == 0) {    /* array of incomplete array */
        ty_t *t = ty;
        do {
            t = t->type;
            assert(t);
            if (t->size == 0)
                break;
        } while(TY_ISARRAY(t));
        if (t->size > 0)
            err_dpos(pos, ERR_TYPE_ARRINCOMP);
    } else if (ty->size == 0)    /* array of incomplete type */
        err_dpos(pos, ERR_TYPE_ARRINCOMP);
    else if (n < 0 || n > TG_LONG_MAX / ty->size) {    /* too big */
        n = 1;
        (void)(err_dpos(pos, ERR_TYPE_BIGARR, (long)n) &&
               err_dpos(pos, ERR_TYPE_BIGARRSTD, (unsigned long)TL_OBJ_STD));
    }
    /* check for TL_OBJ_STD performed in dclr() */

    return type(TY_ARRAY, ty, n*ty->size, ty->align, NULL);
}


/*
 *  decays an array to a pointer;
 *  if ty is not an array, a pointer to ty returned
 */
ty_t *(ty_atop)(ty_t *ty)
{
    assert(ty);
    assert(TY_ISARRAY(ty));

    return ty_ptr(ty->type);
}


/*
 *  finds the (non-array) element type of an array;
 *  if ty is not an array, ty returned
 */
ty_t *(ty_arrelem)(ty_t *ty)
{
    assert(ty);

    while (TY_ISARRAY(ty))
        ty = ty->type;

    return ty;
}


/*
 *  applies qualifiers to a type;
 *  if ty is a funtion or already qualified by op, ty returned;
 */
ty_t *(ty_qual)(int op, ty_t *ty, int dclr, const lmap_t *pos)
{
    assert(op & TY_CONSVOL);
    assert(ty);

    if (TY_ISARRAY(ty))
        ty = type(TY_ARRAY, ty_qual(op, ty->type, dclr, pos), ty->size, ty->align, NULL);
    else if (TY_ISFUNC(ty))
        err_dpos(pos, ERR_TYPE_QUALFUNC, op);
    else if (ty->op & op)
        err_dpos(pos, (dclr)? ERR_TYPE_DUPQUALDCLR: ERR_TYPE_DUPQUAL, op);
    else {
        if (TY_ISQUAL(ty)) {
            op |= ty->op;
            ty = ty->type;
        }
        ty = type(op, ty, ty->size, ty->align, NULL);
    }

    return ty;
}


/*
 *  applies qualifiers to a type depending on conditionals;
 *  arrays and already-qualified types are handled safely
 */
ty_t *(ty_qualc)(int op, ty_t *ty)
{
    ty_t *bty = ty_arrelem(ty);

    assert(!op || !TY_ISFUNC(ty));

    op &= ~bty->op;
    return (op)? ty_qual(op, ty, 0, NULL): ty;
}


/*
 *  constructs a function type;
 *  if ty is a function, a function returning a pointer to ty returned;
 *  if ty is an array, a function returning a pointer to ty->type returned
 */
ty_t *(ty_func)(ty_t *ty, void *proto[], int style, const lmap_t *pos)    /* ty_t */
{
    void **p = NULL;

    assert(ty);

    if (proto)
        for (p = proto; *p && !TY_ISUNKNOWN(T(*p)); p++)
            continue;

    if (TY_ISARRAY(ty)) {
        err_dpos(pos, ERR_TYPE_FUNCARR);
        ty = ty_atop(ty);    /* func returning pointer to element */
    } else if (TY_ISFUNC(ty)) {
        err_dpos(pos, ERR_TYPE_FUNCFUNC);
        ty = ty_ptr(ty);    /* func returning pointer to func */
    }
    ty = type(TY_FUNCTION, ty, 0, 0, NULL);
    ty->u.f.proto = proto;
    ty->u.f.oldstyle = style;
    ty->t.unknown |= (p && *p);

    return ty;
}


/*
 *  returns a return type of a function;
 *  if ty is not a function, ty_inttype returned
 */
ty_t *(ty_freturn)(ty_t *ty)
{
    assert(ty);
    assert(ty_inttype);    /* ensures types initialized */

    if (TY_ISFUNC(ty))
        return ty->type;

    return ty_unknowntype;
}


/*
 *  checks if a function is variadic;
 *  1 returned iff ty is a variadic function
 */
int (ty_variadic)(const ty_t *ty)
{
    assert(ty);
    assert(ty_voidtype);    /* ensures types initialized */

    if (TY_ISFUNC(ty) && ty->u.f.proto) {
        int i;
        for (i = 0; ty->u.f.proto[i]; i++)
            continue;
        return (i > 1 && ty->u.f.proto[i-1] == ty_voidtype);
    }

    return 0;
}


/*
 *  constructs a new struct/union/enum;
 *  if a struct is redefined, a new type is constructed
 */
ty_t *(ty_newstruct)(int ctx, int op, const char *tag, const lmap_t *pos)
{
    sym_t *p;
    int newty = (ctx == '{' || ctx == ';');

    assert(tag);
    assert(pos);

    if (*tag == '\0')
        tag = hash_int(sym_genlab(1));
    else
        if ((p = sym_lookup(tag, sym_type)) != NULL && (!newty || SYM_SAMESCP(p, sym_scope))) {
            if (p->type->op != op)
                (void)(err_dpos(pos, ERR_TYPE_DIFFTAG, p, " a tag") &&
                       err_dpos(p->pos, ERR_PARSE_PREVDECL));
            else if (ctx == '{' && p->f.defined)
                (void)(err_dpos(pos, ERR_TYPE_STRREDEF, p, " a tag") &&
                       err_dpos(p->pos, ERR_PARSE_PREVDEF));
            else
                return p->type;
        } else if (op == TY_ENUM && !p && !newty)
            err_dpos(pos, ERR_TYPE_INVENUM);
    decl_chkid(tag, pos, sym_type, 0);
    p = sym_new(SYM_KTAG, tag, pos);
    p->type = type(op, NULL, 0, 0, p);    /* has circular reference */
    if (p->scope > maxlevel)
        maxlevel = p->scope;

    return p->type;
}


/*
 *  creates a new field for a structure/union;
 *  a duplicated name, if any, is inserted anyway
 */
sym_field_t *(ty_newfield)(const char *name, ty_t *ty, ty_t *fty, const lmap_t *pos)
{
    sym_field_t *p, **q;
    const char *cname;

    assert(ty);
    assert(ty->u.sym);
    assert(fty);
    assert(pos);

    cname = sym_cname(name, 0);
    q = &ty->u.sym->u.s.flist;
    if (!name)
        name = hash_int(sym_genlab(1));
    for (p = *q; p; q=&p->link, p=*q)
        if (p->name == name) {
            (void)(err_dpos(pos, ERR_TYPE_STRDUPMEM, name, "") &&
                   err_dpos(p->pos, ERR_PARSE_PREVDECL));
            name = sym_semigenlab();    /* helps ty_chkfield() */
        } else if (cname && p->cname == cname)
            (void)(err_dpos(pos, ERR_LEX_LONGID, p->name) &&
                   err_dpos(pos, ERR_LEX_LONGIDSTD, (long)TL_INAME_STD) &&
                   err_dpos(p->pos, ERR_PARSE_PREVDECL));
    p = ARENA_CALLOC(strg_perm, 1, sizeof(*p));
    *q = p;
    p->name = name;
    p->cname = cname;
    p->type = fty;
    p->pos = pos;

    return p;
}


/*
 *  checks if two types equal
 */
int (ty_same)(const ty_t *t1, const ty_t *t2)
{
    assert(t1);
    assert(t2);

    if (t1->t.type == t2->t.type)
        return 1;
    if (t1->op != t2->op)
        return 0;
    switch(t1->op) {
#if 0    /* only one type assigned to each type op */
        case TY_UNKNOWN:
        case TY_VOID:
        case TY_INT:
        case TY_UNSIGNED:
        case TY_LONG:
        case TY_ULONG:
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
            assert(!"invalid type configuration -- should never reach here");
            break;
#endif    /* disabled */
        case TY_CHAR:      /* plain/signed/unsigned char */
        case TY_SHORT:     /* signed/unsigned short */
        case TY_ENUM:      /* t1->t.type == t2->t.type iff compatible */
        case TY_STRUCT:    /* t1->t.type == t2->t.type iff compatible */
        case TY_UNION:     /* t1->t.type == t2->t.type iff compatible */
            return 0;
        case TY_POINTER:
            return ty_same(t1->type, t2->type);
        case TY_VOLATILE:    /* equally-qualified */
        case TY_CONST:
        case TY_CONSVOL:
            return ty_same(t1->type, t2->type);
        case TY_ARRAY:
            return (ty_same(t1->type, t2->type) && t1->size == t2->size);
        case TY_FUNCTION:
            return (ty_same(t1->type, t2->type) && t1->u.f.proto == t2->u.f.proto);
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return 0;
}


/*
 *  checks if two types are compatible;
 *  0 returned if not compatible;
 *  1 returned if compatible;
 *  > 1 returned if compatible but enum and int involved
 */
int (ty_equiv)(const ty_t *t1, const ty_t *t2, int ret)
{
    int res;

    assert(t1);
    assert(t2);

    if (t1->t.type == t2->t.type)
        return 1;
    if (!TY_ISQUAL(t1) && !TY_ISQUAL(t2) && CMPENUM(t1, t2))
        return 2;    /* > 1 implies 1 */
    if (t1->op != t2->op)
        return 0;
    /* t1 != t2 but t1->op == t2->op types handled below */
    switch(t1->op) {
#if 0    /* only one type assigned to each type op */
        case TY_UNKNOWN:
        case TY_VOID:
        case TY_INT:
        case TY_UNSIGNED:
        case TY_LONG:
        case TY_ULONG:
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
            assert(!"invalid type configuration -- should never reach here");
            break;
#endif    /* disabled */
        case TY_CHAR:      /* plain/signed/unsigned char */
        case TY_SHORT:     /* signed/unsigned short */
        case TY_ENUM:      /* t1->t.type == t2->t.type iff compatible */
        case TY_STRUCT:    /* t1->t.type == t2->t.type iff compatible */
        case TY_UNION:     /* t1->t.type == t2->t.type iff compatible */
            return 0;
        case TY_POINTER:
            return ty_equiv(t1->type, t2->type, 1);
        case TY_VOLATILE:    /* equally-qualified */
        case TY_CONST:
        case TY_CONSVOL:
            return ty_equiv(t1->type, t2->type, 1);
        case TY_ARRAY:
            if ((res = ty_equiv(t1->type, t2->type, 1)) != 0) {
                if (t1->size == t2->size)
                    return res;
                if (t1->size == 0 || t2->size == 0)
                    return ret * res;
            }
            return 0;
        case TY_FUNCTION:
            if ((res = ty_equiv(t1->type, t2->type, 1)) != 0) {
                void **p1 = t1->u.f.proto,
                     **p2 = t2->u.f.proto;    /* ty_t */
                if (p1 == p2)
                    return res;
                if (p1 && p2) {
                    int res2;
                    for (; *p1 && *p2; p1++, p2++)
                        if ((res2 = ty_equiv(TY_UNQUAL(T(*p1)), TY_UNQUAL(T(*p2)), 1)) == 0)
                            return 0;
                        else
                            res |= res2;
                    if (!*p1 && !*p2)
                        return res;
                } else {
                    if (ty_variadic(p1? t1: t2))
                        return 0;
                    if (!p1)
                        p1 = p2;
                    for (; *p1; p1++) {    /* no need to check > 1; done in funcdefn() */
                        ty_t *ty = TY_UNQUAL(T(*p1));
                        ty_t *pty = ty_apromote(ty);
                        if (!ty_equiv(ty, pty, 1))
                            return 0;
                    }
                    return res;
                }
            }
            return 0;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return 0;    /* never reached without assertion failure */
}


/*
 *  performs the integral promotion;
 *  ASSUMPTION: int represets all small integers
 */
ty_t *(ty_ipromote)(ty_t *ty)
{
    assert(ty);
    assert(ty_inttype);    /* ensures types initialized */

    ty = TY_UNQUAL(ty);

    return (ty->op == TY_CHAR || ty->op == TY_SHORT)? ty_inttype: ty;
}


/*
 *  performs the argument promotion
 */
ty_t *(ty_apromote)(ty_t *ty)
{
    assert(ty);
    assert(ty_doubletype);    /* ensures types initialized */

    return (TY_ISFLOAT(ty))? ty_doubletype: ty_ipromote(ty);
}


/*
 *  constructs a composite type of two compatible types;
 *  ASSUMPTION: the composite type of enum and compatible integer is integer
 */
ty_t *(ty_compose)(ty_t *t1, ty_t *t2)
{
    assert(t1);
    assert(t2);

    if (CMPENUM(t1, t2))
        return (TY_ISENUM(t1))? t2: t1;    /* enum + int = int */
    if (t1 == t2)
        return t1;
    if (t1->t.type == t2->t.type)
        return t1->t.type;

    /* ty_equiv(t1, t2) should hold */
    assert(t1->op == t2->op);

    switch(t1->op) {
#if 0    /* if ty_equiv() holds, t1 == t2 or t1->t.type == t2->t.type */
        case TY_UNKNOWN:
        case TY_VOID:
        case TY_CHAR:
        case TY_SHORT:
        case TY_INT:
        case TY_UNSIGNED:
        case TY_LONG:
        case TY_ULONG:
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
        case TY_ENUM:
        case TY_STRUCT:
        case TY_UNION:
            assert(!"invalid type configuration -- should never reach here");
            break;
#endif    /* disabled */
        case TY_POINTER:
            return ty_ptr(ty_compose(t1->type, t2->type));
        case TY_CONST:
        case TY_VOLATILE:
        case TY_CONSVOL:
            t1 = ty_qualc(t1->op, ty_compose(t1->type, t2->type));
            return t1;
        case TY_ARRAY:
            {
                ty_t *ty = ty_compose(t1->type, t2->type);
                if (t1->size > 0) {
                    assert(t2->size == 0 || t1->size == t2->size);
                    assert(t1->type->size > 0);
                    assert(ty->align == t1->align);    /* ok to remove align from array()? */
                    t1 = ty_array(ty, t1->size/t1->type->size, NULL);
                } else if (t2->size > 0) {
                    assert(t2->type->size > 0);
                    assert(ty->align == t2->align);    /* ok to remove align from array()? */
                    t1 = ty_array(ty, t2->size/t2->type->size, NULL);
                } else
                    t1 = ty_array(ty, 0, NULL);
            }
            return t1;
        case TY_FUNCTION:
            {
                void **p1 = t1->u.f.proto,
                     **p2 = t2->u.f.proto;    /* ty_t */
                ty_t *ty = ty_compose(t1->type, t2->type);
                alist_t *tlist = NULL;
                assert(!TY_ISFUNC(ty) && !TY_ISARRAY(ty));
                if (!p1 && !p2)
                    t1 = ty_func(ty, NULL, 1, NULL);
                else if (p1 && !p2)
                    t1 = ty_func(ty, p1, t1->u.f.oldstyle, NULL);
                else if (!p1 && p2)
                    t1 = ty_func(ty, p2, t2->u.f.oldstyle, NULL);
                else {
                    for (; *p1 && *p2; p1++, p2++)
                        tlist = alist_append(tlist,
                                    ty_qualc((T(*p1)->op | T(*p2)->op) & TY_CONSVOL,
                                             ty_compose(TY_UNQUAL(T(*p1)), TY_UNQUAL(T(*p2)))),
                                    strg_perm);
                    assert(!*p1 && !*p2);    /* # of parameters matches */
                    t1 = ty_func(ty, alist_toarray(tlist, strg_perm), 0, NULL);
                }
            }
            return t1;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return NULL;    /* never reached without assertion failure */
}


/*
 *  inspects if a type has parameter information;
 *  0 returned iff ty contains a function type that has no parameter info;
 *  called for each function parameter when parsing them;
 *  called for each struct/union member when parsing them;
 *  called for type names because they have no declarations
 */
int (ty_hasproto)(const ty_t *ty)
{
    int spec;

    assert(ty);

    spec = !!(ty->t.name) << 1;

    switch(ty->op) {
        case TY_CONST:
        case TY_VOLATILE:
        case TY_CONSVOL:
        case TY_POINTER:
        case TY_ARRAY:
            return spec | ty_hasproto(ty->type);
        case TY_FUNCTION:
            return spec | (ty->u.f.proto && ty_hasproto(ty->type));
        case TY_UNKNOWN:
        case TY_STRUCT:
        case TY_UNION:
        case TY_CHAR:
        case TY_SHORT:
        case TY_INT:
        case TY_UNSIGNED:
        case TY_LONG:
        case TY_ULONG:
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
        case TY_ENUM:
        case TY_VOID:
            return spec | 1;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return spec | 0;
}


/*
 *  finds a truncated name in a field list
 */
static sym_field_t *iscfield(const char *name, sym_field_t *flist)
{
    if ((name = sym_cname(name, 0)) != NULL) {
        for (; flist; flist = flist->link)
            if (flist->cname == name)
                break;
    } else
        flist = NULL;

    return flist;
}


/*
 *  finds a name in a field list
 */
static sym_field_t *isfield(const char *name, sym_field_t *flist)
{
    assert(name);

    for (; flist; flist = flist->link)
        if (flist->name == name)
            break;

    return flist;
}


/*
 *  checks ty for ambiguous inherited members;
 *  returns an augmented set of members
 */
static sym_field_t *check(ty_t *ty, ty_t *top, sym_field_t *inherited)
{
    sym_field_t *p;

    assert(ty);
    assert(!TY_ISQUAL(ty));
    assert(TY_ISSTRUNI(ty));

    for (p = ty->u.sym->u.s.flist; p; p = p->link)
        if (p->name) {
            sym_field_t *q;
            if ((q = isfield(p->name, inherited)) != NULL)
                (void)(err_dpos(p->pos, ERR_TYPE_STRAMBMEM, p->name, "") &&
                       err_dpos(q->pos, ERR_TYPE_SEEMEMBER, top));
            else {
                if ((q = iscfield(p->name, inherited)) != NULL)
                    (void)(err_dpos(p->pos, ERR_LEX_LONGID) &&
                           err_dpos(p->pos, ERR_LEX_LONGIDSTD, (long)TL_INAME_STD) &&
                           err_dpos(q->pos, ERR_LEX_SEEID, q->name));
                q = ARENA_ALLOC(strg_func, sizeof(*q));
                *q = *p;
                q->link = inherited;
                inherited = q;
            }
            if (!p->incomp)
                ty_chkfield(p->type);
        } else
            inherited = check(TY_UNQUAL(p->type), top, inherited);

    return inherited;
}


/*
 *  initiates check() to check ambiguous inherited members in struct/union
 */
void (ty_chkfield)(ty_t *ty)
{
    assert(ty);

    if (TY_ISSTRUNI(ty))
        check(TY_UNQUAL(ty), ty, NULL);
}


/*
 *  finds a member name in ty
 */
sym_field_t *(ty_fieldref)(const char *name, ty_t *ty)
{
    sym_field_t *p;

    assert(ty);
    assert(TY_ISSTRUNI(ty));

    if ((p = isfield(name, TY_UNQUAL(ty)->u.sym->u.s.flist)) != NULL)
        return p;
    if (main_opt()->std == 0 || main_opt()->std > 2)
        for (p = TY_UNQUAL(ty)->u.sym->u.s.flist; p; p = p->link) {
            sym_field_t *q;
            if (!p->name && TY_ISSTRUNI(p->type) && (q = ty_fieldref(name, p->type)) != NULL) {
                static sym_field_t f;
                f = *q;
                f.offset = p->offset + q->offset;
                return &f;
            }
        }

    return NULL;
}


/*
 *  finds the signed counter of an integer type
 */
ty_t *(ty_scounter)(ty_t *ty)
{
    assert(ty);
    assert(TY_ISINTEGER(ty));
    assert(!TY_ISQUAL(ty));
    assert(ty_ipromote(ty)->t.type == ty->t.type);
    assert(ty_inttype);    /* ensures types initialized */

    ty = TY_RMQENUM(ty);
    switch(ty->op) {
        case TY_INT:
        case TY_LONG:
            break;
        case TY_UNSIGNED:
            ty = ty_inttype;
            break;
        case TY_ULONG:
            ty = ty_longtype;
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return ty;
}


/*
 *  finds the unsigned counter of an integer type
 */
ty_t *(ty_ucounter)(ty_t *ty)
{
    assert(ty);
    assert(TY_ISINTEGER(ty));
    assert(!TY_ISQUAL(ty));
    assert(ty_ipromote(ty)->t.type == ty->t.type);
    assert(ty_unsignedtype);    /* ensures types initialized */

    ty = TY_RMQENUM(ty);
    switch(ty->op) {
        case TY_INT:
            ty = ty_unsignedtype;
            break;
        case TY_LONG:
            ty = ty_ulongtype;
            break;
        case TY_UNSIGNED:
        case TY_ULONG:
            break;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return ty;
}


/*
 *  checks if a type contains a typedef synonym;
 *  no checks for parameter types
 */
int (ty_hastypedef)(const ty_t *ty)
{
    assert(ty);

    do {
        if (ty->t.name)
            return 1;
        ty = ty->type;
    } while(ty);

    return 0;
}


/*
 *  provides a buffer for making up strings
 */
static const char *sout(const char *s, ...)
{
    static char buf[150+3+1];

    va_list ap;
    char *p = buf, *s0 = NULL;

    va_start(ap, s);
    while (s) {
        while (*s && p-buf < sizeof(buf)-1-3)
            *p++ = *s++;
        *p = '\0';
        s0 = p;
        if (*s) {    /* buffer full */
            strcpy(s0, "...");
            break;
        }
        s = va_arg(ap, const char *);
    }
    va_end(ap);

    return hash_string(buf);    /* returned buffer to be given sout */
}


/*
 *  returns a string for basic types (except enum)
 */
static const char *btname(const ty_t *ty)
{
    assert(ty);

    ty = ty->t.type;

    assert(!TY_ISQUAL(ty));
    assert(ty->u.sym);
    assert(ty->u.sym->name);
    assert(ty_uchartype);    /* ensures types initialized */

    switch(ty->op) {
        case TY_ULONG:
            return ty->u.sym->name;
        case TY_CHAR:    /* char, signed char, unsigned char */
            return (ty == ty_uchartype)? "unsigned char":
                   (ty == ty_schartype)? "signed char": "char";
        case TY_SHORT:    /* short, unsigned short */
            return (ty == ty_ushorttype)? "unsigned short": "short";
#if 0    /* only one type assigned to each type op */
        case TY_UNKNOWN:
        case TY_VOID:
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
        case TY_INT:
        case TY_UNSIGNED:
        case TY_LONG:
#endif    /* disabled */
        default:
            return ty->u.sym->name;
    }

    /* assert(!"impossible control flow -- should never reach here");
       return ""; */
}


/*
 *  constructs a string representation for a type;
 *  might intrude on lmap_out() if given as arguments to the same call;
 *  see also lmap_out()
 */
const char *(ty_outtype)(const ty_t *ty, int exp)
{
    const char *s = "";

    assert(ty);
    assert(ty_voidtype);    /* ensures types initialized */

    for (; ty; ty = ty->type) {
        if (!exp && ty->t.name)
            return (ty->size > 0 || TY_ISVOID(ty))?
                       sout(s, ty->t.name, NULL): sout(s, "incomplete ", ty->t.name, NULL);
        switch(ty->op) {
            case TY_CONST:
            case TY_VOLATILE:
                s = sout(s, clx_name[ty->op], " ", NULL);
                assert(ty->type);
                break;
            case TY_CONSVOL:
                s = sout(s, clx_name[TY_CONST], " ", clx_name[TY_VOLATILE], " ", NULL);
                assert(ty->type);
                break;
            case TY_STRUCT:
            case TY_UNION:
            case TY_ENUM:
                {
                    assert(ty->u.sym);
                    if (ty->size == 0) {
                        assert(!TY_ISENUM(ty));
                        s = sout(s, "incomplete ", NULL);
                    }
                    assert(ty->u.sym->name);
                    s = (GENNAME(ty->u.sym->name))?
                            sout(s, clx_name[ty->op], " defined at ",
                                    lmap_out(ty->u.sym->pos), NULL):
                            sout(s, clx_name[ty->op], " ", ty->u.sym->name, NULL);
                    if (ty->op == TY_ENUM)
                        ty = ty->type;
                    assert(!ty->type);
                }
                break;
            case TY_UNKNOWN:
            case TY_VOID:
            case TY_FLOAT:
            case TY_DOUBLE:
            case TY_LDOUBLE:
            case TY_CHAR:
            case TY_SHORT:
            case TY_INT:
            case TY_UNSIGNED:
            case TY_LONG:
            case TY_ULONG:
                s = sout(s, btname(ty), NULL);
                assert(!ty->type);
                break;
            case TY_POINTER:
                s = sout(s, "pointer to ", NULL);
                assert(ty->type);
                break;
            case TY_FUNCTION:
                s = sout(s, "function(", NULL);
                if (ty->u.f.proto) {
                    int i;
                    if (ty->u.f.proto[0]) {
                        s = sout(s, ty_outtype(T(ty->u.f.proto[0]), exp), NULL);
                        for (i = 1; ty->u.f.proto[i]; i++)
                            if (ty->u.f.proto[i] == ty_voidtype)
                                s = sout(s, ", ...", NULL);
                            else
                                s = sout(s, ", ", ty_outtype(T(ty->u.f.proto[i]), exp), NULL);
                    } else
                        s = sout(s, "void", NULL);
                }
                s = sout(s, ") returning ", NULL);
                assert(ty->type);
                break;
            case TY_ARRAY:
                if (ty->size > 0) {
                    s = sout(s, "array ", NULL);
                    do {
                        assert(ty->type);
                        assert(ty->type->size > 0);
                        s = sout(s, (sprintf(num, "[%ld]", ty->size/ty->type->size), num), NULL);
                    } while(TY_ISARRAY(ty->type) && (ty = ty->type) != NULL);
                } else
                    s = sout(s, "incomplete array", NULL);
                s = sout(s, " of ", NULL);
                assert(ty->type);
                break;
            default:
                assert(!"invalid type operator -- should never reach here");
                break;
        }
    }

    return s;
}


/*
 *  constructs a C declaration for ty
 */
const char *(ty_outdecl)(const ty_t *ty, const char *s, int *pa, int exp)
{
    assert(ty);
    assert(s);
    assert(ty_voidtype);    /* ensures types initialized */

    *pa = 0;
    for (; ty; ty = ty->type) {
        if (!exp && ty->t.name)
            return (*s)? sout(ty->t.name, " ", s, NULL): ty->t.name;
        switch(ty->op) {
            /* types in specifier */
            case TY_STRUCT:
            case TY_UNION:
            case TY_ENUM:
                assert(ty->u.sym);
                assert(ty->u.sym->name);
                if (GENNAME(ty->u.sym->name)) {
                    *pa = 1;
                    return (*s)? sout(clx_name[ty->op], " ", s, NULL): clx_name[ty->op];
                }
                return (*s)? sout(clx_name[ty->op], " ", ty->u.sym->name, " ", s, NULL):
                             sout(clx_name[ty->op], " ", ty->u.sym->name, NULL);
            case TY_UNKNOWN:
                return (*s)? sout("<unknown type> ", s, NULL): "<unknown type>";
            case TY_VOID:
            case TY_FLOAT:
            case TY_DOUBLE:
            case TY_LDOUBLE:
            case TY_CHAR:
            case TY_SHORT:
            case TY_INT:
            case TY_UNSIGNED:
            case TY_LONG:
            case TY_ULONG:
                return (*s)? sout(btname(ty), " ", s, NULL): btname(ty);
            /* types in declarator */
            case TY_POINTER:
                assert(ty->type);
                s = (TY_ISARRAY(ty->type) || TY_ISFUNC(ty->type))? sout("(*", s, ")", NULL):
                                                                   sout("*", s, NULL);
                break;
            case TY_FUNCTION:
                if (!ty->u.f.proto)
                    s = sout(s, "()", NULL);
                else {
                    int i;
                    s = sout(s, "(", NULL);
                    if (ty->u.f.proto[0]) {
                        s = sout(s, ty_outdecl(T(ty->u.f.proto[0]), "", pa, exp), NULL);
                        for (i = 1; ty->u.f.proto[i]; i++)
                            s = (ty->u.f.proto[i] == ty_voidtype)?
                                    sout(s, ", ...", NULL):
                                    sout(s, ", ", ty_outdecl(T(ty->u.f.proto[i]), "", pa, exp),
                                         NULL);
                    } else
                        s = sout(s, "void", NULL);
                    s = sout(s, ")", NULL);
                }
                break;
            case TY_ARRAY:
                assert(ty->type);
                if (ty->size > 0) {
                    assert(ty->type->size > 0);
                    s = sout(s, "[", (sprintf(num, "%ld", ty->size/ty->type->size), num), "]",
                             NULL);
                } else
                    s = sout(s, "[]", NULL);
                break;
            /* types in both */
            case TY_CONST:
            case TY_VOLATILE:
                if (TY_ISPTR(ty->type))
                    s = sout(" ", clx_name[ty->op], (*s)? " ": "", s, NULL);
                else
                    return sout(clx_name[ty->op], " ", ty_outdecl(ty->type, s, pa, exp), NULL);
                break;
            case TY_CONSVOL:
                if (TY_ISPTR(ty->type))
                    s = sout(clx_name[TY_CONST], " ", clx_name[TY_VOLATILE], (*s)? " ": "", s,
                             NULL);
                else
                    return sout(clx_name[TY_CONST], " ", clx_name[TY_VOLATILE], " ",
                                ty_outdecl(ty->type, s, pa, exp), NULL);
                break;
            default:
                assert(!"invalid type operator -- should never reach here");
                return NULL;
        }
    }

    assert(!"invalid type structure -- should never reach here");
    return NULL;
}


/*
 *  returns a string representing the category of ty
 */
const char *(ty_outcat)(const ty_t *ty)
{
    int op;

    assert(ty);

    ty = TY_UNQUAL(ty);
    switch(op = ty->op) {
        case TY_CONST:
        case TY_VOLATILE:
        case TY_CONSVOL:
            assert(!"invalid type structure -- should never reach here");
            break;
        case TY_STRUCT:
        case TY_UNION:
        case TY_ENUM:
            return clx_name[op];
        case TY_UNKNOWN:
            if (ty->t.name)
                return ty->t.name;
        case TY_VOID:
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
        case TY_CHAR:
        case TY_SHORT:
        case TY_INT:
        case TY_UNSIGNED:
        case TY_LONG:
        case TY_ULONG:
            return btname(ty);
        case TY_POINTER:
            return "pointer";
        case TY_FUNCTION:
            return "function";
        case TY_ARRAY:
            return "array";
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return NULL;
}

/* end of ty.c */
