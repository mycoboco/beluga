/*
 *  type system
 */

#ifndef TY_H
#define TY_H


typedef struct ty_t ty_t;    /* used in sym.h through lex.h */


#include "lex.h"
#include "lmap.h"
#include "sym.h"


/* type operators */
enum {
    TY_UNKNOWN = 0,
#define LEX2TY(t) TY_##t = LEX_##t
    LEX2TY(FLOAT),
    LEX2TY(DOUBLE),
    LEX2TY(LDOUBLE),
    LEX2TY(CHAR),
    LEX2TY(SHORT),
    LEX2TY(INT),
    LEX2TY(UNSIGNED),
    LEX2TY(LONG),
    LEX2TY(ULONG),
#ifdef SUPPORT_LL
    LEX2TY(LLONG),
    LEX2TY(ULLONG),
#endif    /* SUPPORT_LL */
    LEX2TY(POINTER),
    LEX2TY(VOID),
    LEX2TY(STRUCT),
    LEX2TY(UNION),
    LEX2TY(FUNCTION),
    LEX2TY(ARRAY),
    LEX2TY(ENUM),
    LEX2TY(CONST),
    LEX2TY(VOLATILE),
#undef LEX2TY
    TY_CONSVOL = TY_CONST | TY_VOLATILE
};

/* type;
   ty_t pointer not const-qualified in most cases due to type implementation design;
   ASSUMPTION: alignment restriction is represented by integral alignment factor */
struct ty_t {
    short op;             /* type operator (TY_*) */
    short align;          /* alignment factor */
    ssz_t size;           /* size in bytes */
    struct ty_t *type;    /* type operand */
    struct {
        const char *name;       /* typedef name if any */
        struct ty_t *type;      /* original type; may embed typedef */
        unsigned plain:   1;    /* true if plain int denoted */
        unsigned unknown: 1;    /* true if unknown type (even when not typedef) */
    } t;                        /* typedef synonym */
    union {
        sym_t *sym;    /* related symbol */
        struct {
            unsigned oldstyle: 1;    /* true if non-prototype */
            unsigned implint:  1;    /* true if return type is implicit int */
            void **proto;            /* (ty_t) array of parameter types */
            const lmap_t *pos;       /* locus for prototype */
        } f;                         /* function */
    } u;
};


/* built-in types + pointer to void */
extern ty_t *ty_unknowntype;     /* unknown type */
extern ty_t *ty_chartype;        /* plain char */
extern ty_t *ty_schartype;       /* signed char */
extern ty_t *ty_uchartype;       /* unsigned char */
extern ty_t *ty_shorttype;       /* short */
extern ty_t *ty_ushorttype;      /* unsigned short */
extern ty_t *ty_inttype;         /* int */
extern ty_t *ty_unsignedtype;    /* unsigned */
extern ty_t *ty_longtype;        /* long */
extern ty_t *ty_ulongtype;       /* unsigned long */
#ifdef SUPPORT_LL
extern ty_t *ty_llongtype;       /* long long */
extern ty_t *ty_ullongtype;      /* unsigned long long */
#endif    /* SUPPORT_LL */
extern ty_t *ty_floattype;       /* float */
extern ty_t *ty_doubletype;      /* double */
extern ty_t *ty_ldoubletype;     /* long double */
extern ty_t *ty_voidtype;        /* void */
extern ty_t *ty_voidptype;       /* pointer to void */

/* frequently used types; point to built-in types */
extern ty_t *ty_sizetype;        /* size_t */
extern ty_t *ty_ptrdifftype;     /* ptrdiff_t */
extern ty_t *ty_ptruinttype;     /* unsigned integer type for pointer arithmetic */
extern ty_t *ty_ptrsinttype;     /* signed integer type for pointer arithmetic */
extern ty_t *ty_wuchartype;      /* unsigned counterpart of wchar_t */
extern ty_t *ty_wchartype;       /* wchar_t */
extern ty_t *ty_winttype;        /* wint_t */


void ty_init(void);
void ty_rmtype(int);
ty_t *ty_ptr(ty_t *);
ty_t *ty_deref(ty_t *);
ty_t *ty_array(ty_t *, ssz_t, const lmap_t *);
ty_t *ty_atop(ty_t *);
ty_t *ty_arrelem(ty_t *);
ty_t *ty_qual(int, ty_t *, int, const lmap_t *);
ty_t *ty_qualc(int, ty_t *);
ty_t *ty_func(ty_t *, void *[], int, const lmap_t *);    /* ty_t */
ty_t *ty_freturn(ty_t *);
int ty_variadic(const ty_t *);
ty_t *ty_newstruct(int, int, const char *, const lmap_t *);
sym_field_t *ty_newfield(const char *, ty_t *, ty_t *, const lmap_t *);
int ty_same(const ty_t *, const ty_t *);
int ty_equiv(const ty_t *, const ty_t *, int);
ty_t *ty_ipromote(ty_t *);
ty_t *ty_apromote(ty_t *);
ty_t *ty_compose(ty_t *, ty_t *);
int ty_hasproto(const ty_t *);
void ty_chkfield(ty_t *);
sym_field_t *ty_fieldref(const char *, ty_t *);
ty_t *ty_scounter(ty_t *);
ty_t *ty_ucounter(ty_t *);
int ty_hastypedef(const ty_t *);
const char *ty_outtype(const ty_t *, int);
const char *ty_outdecl(const ty_t *, const char *, int *, int);
const char *ty_outcat(const ty_t *);


/* type predicates; to ignore type qualifiers;
   ASSUMPTION: enum is always int */
#define TY_ISUNKNOWN(ty) ((ty)->t.unknown)
#define TY_ISFLOAT(t)    (TY_UNQUAL(t)->op == TY_FLOAT)
#define TY_ISDOUBLE(t)   (TY_UNQUAL(t)->op == TY_DOUBLE)
#define TY_ISLDOUBLE(t)  (TY_UNQUAL(t)->op == TY_LDOUBLE)
#define TY_ISCHAR(t)     (TY_UNQUAL(t)->op == TY_CHAR)        /* plain/(un)signed char */
#define TY_ISSHORT(t)    (TY_UNQUAL(t)->op == TY_SHORT)       /* (un)signed short */
#define TY_ISINT(t)      (TY_UNQUAL(t)->op == TY_INT)         /* plain/signed int */
#define TY_ISUNSIGNED(t) (TY_UNQUAL(t)->op == TY_UNSIGNED)    /* unsigned int */
#define TY_ISLONG(t)     (TY_UNQUAL(t)->op == TY_LONG)
#define TY_ISULONG(t)    (TY_UNQUAL(t)->op == TY_ULONG)
#ifdef SUPPORT_LL
#define TY_ISLLONG(t)    (TY_UNQUAL(t)->op == TY_LLONG)
#define TY_ISULLONG(t)   (TY_UNQUAL(t)->op == TY_ULLONG)
#endif     /* SUPPORT_LL */
#define TY_ISPTR(t)      (TY_UNQUAL(t)->op == TY_POINTER)
#define TY_ISVOID(t)     (TY_UNQUAL(t)->op == TY_VOID)
#define TY_ISSTRUCT(t)   (TY_UNQUAL(t)->op == TY_STRUCT)
#define TY_ISUNION(t)    (TY_UNQUAL(t)->op == TY_UNION)
#define TY_ISFUNC(t)     (TY_UNQUAL(t)->op == TY_FUNCTION)
#define TY_ISARRAY(t)    (TY_UNQUAL(t)->op == TY_ARRAY)
#define TY_ISENUM(t)     (TY_UNQUAL(t)->op == TY_ENUM)

#ifdef SUPPORT_LL
#define TY_ISINTEGER(t)  (TY_UNQUAL(t)->op >= TY_CHAR &&    \
                          TY_UNQUAL(t)->op <= TY_ULLONG)     /* integers + enum */
#define TY_ISUNSIGN(t)   (TY_UNQUAL(t)->op == TY_UNSIGNED ||    \
                          TY_UNQUAL(t)->op == TY_ULONG ||       \
                          TY_UNQUAL(t)->op == TY_ULLONG)     /* unsigned types */
#else     /* !SUPPORT_LL */
#define TY_ISINTEGER(t)  (TY_UNQUAL(t)->op >= TY_CHAR &&    \
                          TY_UNQUAL(t)->op <= TY_ULONG)      /* integers + enum */
#define TY_ISUNSIGN(t)   (TY_UNQUAL(t)->op == TY_UNSIGNED ||    \
                          TY_UNQUAL(t)->op == TY_ULONG)      /* unsigned types */
#endif    /* SUPPORT_LL */
#define TY_ISSMALLINT(t) (TY_UNQUAL(t)->op >= TY_CHAR &&    \
                          TY_UNQUAL(t)->op < TY_INT)         /* small integers */
#define TY_ISFP(t)       (TY_UNQUAL(t)->op > 0 &&    \
                          TY_UNQUAL(t)->op <= TY_LDOUBLE)    /* floating-point */
#ifdef SUPPORT_LL
#define TY_ISARITH(t)    (TY_UNQUAL(t)->op > 0 &&    \
                          TY_UNQUAL(t)->op <= TY_ULLONG)     /* + enum */
#else    /* !SUPPORT_LL */
#define TY_ISARITH(t)    (TY_UNQUAL(t)->op > 0 &&    \
                          TY_UNQUAL(t)->op <= TY_ULONG)      /* + enum */
#endif    /* SUPPORT_LL */
#define TY_ISSCALAR(t)   (TY_UNQUAL(t)->op > 0 &&    \
                          TY_UNQUAL(t)->op <= TY_POINTER)    /* + enum */
#define TY_ISSTRUNI(t)   (TY_UNQUAL(t)->op == TY_STRUCT ||    \
                          TY_UNQUAL(t)->op == TY_UNION)      /* struct or union */
#define TY_ISVOIDP(t)    (TY_ISPTR(t) && TY_ISVOID((t)->type))

#define TY_ISQUAL(t)      ((t)->op >= TY_CONST)
#define TY_ISCONST(t)     ((t)->op & TY_CONST)
#define TY_ISVOLATILE(t)  ((t)->op & TY_VOLATILE)
#define TY_HASCONST(t)    (TY_ISSTRUNI(t) && TY_UNQUAL(t)->u.sym->u.s.cfield)
#define TY_HASVOLATILE(t) (TY_ISSTRUNI(t) && TY_UNQUAL(t)->u.sym->u.s.vfield)

/* strips qualifiers */
#define TY_UNQUAL(t) (TY_ISQUAL(t)? (t)->type: (t))

/* removes enums and qualifiers */
#define TY_RMQENUM(t) (TY_ISQUAL(t)?                                                    \
                           (((t)->type->op == TY_ENUM)? (t)->type->type: (t)->type):    \
                           (((t)->op == TY_ENUM)? (t)->type: (t)))

/* checks if signed char */
#define TY_ISSCHAR(ty) (((ty)->t.type == ty_chartype && !main_opt()->uchar) ||    \
                        (ty)->t.type == ty_schartype)


#endif    /* TY_H */

/* end of ty.h */
