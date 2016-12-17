/*
 *  symbol table
 */

#ifndef SYM_H
#define SYM_H

#include "alist.h"
#include "common.h"


/* sym_val_t used in tree.h through dag.h */

/* value;
   ASSUMPTION: fp types of the host are same as those of the target;
   ASSUMPTION: long on the host can represent all signed integers on the target;
   ASSUMPTION: unsigned long on the host can represent all unsigned integers on the target */
typedef union sym_val_t {
    sx_t s;            /* for signed integers */
    ux_t u;            /* for unsigned integers; used as key when hashing */
    float f;           /* float */
    double d;          /* double */
    long double ld;    /* long double */
    const void *hp;    /* pointer value on the host */
    ux_t tp;           /* array, function, pointer on the target */
} sym_val_t;

typedef struct sym_t sym_t;                /* used in lex.h through dag.h */
typedef struct sym_field_t sym_field_t;    /* used in ty.h through dag.h */


#include "dag.h"
#include "lmap.h"
#include "tree.h"
#include "ty.h"

#include "cfg.h"


typedef struct sym_tab_t sym_tab_t;        /* symbol table; opaque type */

/* struct/union member */
struct sym_field_t {
    const char *name;            /* name of field */
    const char *cname;           /* name for conflict detection */
    ty_t *type;                  /* type of field */
    const lmap_t *pos;           /* location of definition */
    long offset;                 /* byte offset */
    long bitsize;                /* size of bit-field */
    short lsb;                   /* lsb position of bit-field */
    unsigned incomp: 1;          /* incomplete member;
                                    used to avoid infinite recursion in ty_chkfield() */
    struct sym_field_t *link;    /* next member */
};

/* type list for sym_extern entries */
typedef struct sym_tylist_t {
    ty_t *type;                   /* type of symbol */
    const lmap_t *pos;            /* location of definition */
    struct sym_tylist_t *next;    /* next entry */
} sym_tylist_t;

/* symbol entry;
   TODO: add flags to check the symbol process */
struct sym_t {
    const char *name;        /* symbol name */
    int scope;               /* scope: const, label, global, param, local, local+k */
    short sclass;            /* storage class specifier */
    ty_t *type;              /* type of symbol */
    sym_tylist_t *tylist;    /* type list */
    const lmap_t *pos;       /* location of definition */
    float ref;               /* approx. # of references for obj/func/label */
    struct sym_t *up;        /* link to symbol installed before */
    alist_t *use;            /* list of use places */
    struct {
        unsigned wregister: 1;    /* declared with register or labels are used */
        unsigned outofline: 1;    /* out-of-line constant or symbol from constant */
        unsigned structarg: 1;    /* struct argument */
        unsigned addressed: 1;    /* address has been taken */
        unsigned computed:  1;    /* computed address symbol (symbol+-constant) */
        unsigned temporary: 1;    /* temporary symbol */
        unsigned defined:   1;    /* true if symbol defined properly */
        unsigned reference: 1;    /* true if symbol referenced */
        unsigned set:       2;    /* 1: symbol explicitly set, 2: probably set */
    } f;
    union {
        struct {
            int label;                  /* label # */
            struct sym_t *equatedto;    /* link to equivalent label */
        } l;                            /* label */
        struct {
            unsigned cfield: 1;    /* member has const */
            unsigned vfield: 1;    /* member has volatile */
            sym_field_t *flist;    /* member list */
            alist_t *blist;        /* list of typedefs for incomplete type */
        } s;                       /* struct/union type */
        long value;    /* value for enum constant */
        void **idlist;    /* (sym_t) enumerators for enum type */
        struct {
            sym_val_t v;          /* value */
            struct sym_t *loc;    /* symbol of object to contain constant */
        } c;                      /* constant */
        struct {
            const lmap_t *pt;    /* location of definition; used for profiling */
            int label;           /* label for function exit */
            int ncall;           /* # of calls made in function */
        } f;                     /* function */
        int seg;    /* global; segment where global defined */
        struct {
            dag_node_t *cse;    /* node to compute value of cse */
        } t;                    /* temporary for cse */
        struct {
            sym_val_t max;    /* max. value */
            sym_val_t min;    /* min. value */
        } lim;                /* limit values for primitive types */
        tree_t *bt;    /* base tree for address symbol */
    } u;

    cfg_sym_t x;    /* extension for back-end */
};

/* symbol kinds to control sym_new() */
enum {
    SYM_KORDIN = 1,    /* local and parameter */
    SYM_KENUM,         /* enum constant */
    SYM_KTYPEDEF,      /* typedef name */
    SYM_KGLOBAL,       /* global */
    SYM_KEXTERN,       /* local extern */
    SYM_KLABEL,        /* source-code label */
    SYM_KTAG,          /* tag */
    SYM_KTYPE,         /* type */
    SYM_KTEMPB,        /* temporary symbol by back-end */
    SYM_KTEMP,         /* temporary symbol */
    SYM_KGEN,          /* generated symbol */
    SYM_KADDR          /* address symbol */
};

/* scope values */
enum {
    SYM_SCONST = 1,     /* constant; used only for sym_const */
    SYM_SLABEL,         /* label; used only for labels */
    SYM_SGLOBAL,        /* file scope */
    SYM_SPARAM,         /* parameter (incl. prototype scope) */
    SYM_SLOCAL          /* block scope */
};


extern sym_tab_t *sym_const;     /* constants */
extern sym_tab_t *sym_extern;    /* identifiers declared with extern */
extern sym_tab_t *sym_ident;     /* ordinary identifiers */
extern sym_tab_t *sym_global;    /* ordinary identifiers at file scope */
extern sym_tab_t *sym_type;      /* type tags */
extern sym_tab_t *sym_label;     /* generated labels */

extern int sym_scope;    /* current scope */


sym_tab_t *sym_table(sym_tab_t *, int);
void sym_foreach(sym_tab_t *, int, void (*)(sym_t *, void *), void *);
void sym_enterscope(void);
void sym_exitscope(void);
sym_t *sym_lookup(const char *, sym_tab_t *);
const char *sym_cname(const char *, int);
sym_t *sym_clookup(const char *, sym_tab_t *, int);
sym_t *sym_new(int, ...);
sym_t *sym_findlabel(int);
sym_t *sym_findconst(ty_t *, sym_val_t);
sym_t *sym_findint(long);
int sym_genlab(int);
const char *sym_semigenlab(void);
void sym_use(sym_t *, const lmap_t *);
sym_tylist_t *sym_tylist(sym_tylist_t *, sym_t *);
int sym_sextend(int, sym_field_t *);
const char *sym_vtoa(const ty_t *, sym_val_t);


/* inspects bit-field */
#define SYM_FLDSIZE(p)  ((p)->bitsize)
#define SYM_FLDRIGHT(p) ((p)->lsb - 1)
#define SYM_FLDLEFT(p)  (TG_CHAR_BIT*(p)->type->size - SYM_FLDSIZE(p) - SYM_FLDRIGHT(p))
#define SYM_FLDMASK(p)  (~(~(unsigned long)0 << SYM_FLDSIZE(p)))

#define sym_ref(p, w) ((p)->ref += (w))    /* marks symbol as (un)referenced */

/* mimics integer conversions on the target;
   ASSUMPTION: 2sC for signed integers assumed;
   ASSUMPTION: signed integers are compatible with unsigned ones on the host */
#define SYM_CROPSC(n) ((long)((SYM_CROPUC(n) > TG_SCHAR_MAX)?                              \
                          (~(unsigned long)TG_UCHAR_MAX)|SYM_CROPUC(n): SYM_CROPUC(n)))
#define SYM_CROPSS(n) ((long)((SYM_CROPUS(n) > TG_SHRT_MAX)?                               \
                          (~(unsigned long)TG_USHRT_MAX)|SYM_CROPUS(n): SYM_CROPUS(n)))
#define SYM_CROPSI(n) ((long)((SYM_CROPUI(n) > TG_INT_MAX)?                               \
                          (~(unsigned long)TG_UINT_MAX)|SYM_CROPUI(n): SYM_CROPUI(n)))
#define SYM_CROPSL(n) ((long)((SYM_CROPUL(n) > TG_LONG_MAX)?                               \
                          (~(unsigned long)TG_ULONG_MAX)|SYM_CROPUL(n): SYM_CROPUL(n)))
#define SYM_CROPUC(n) (((unsigned long)(n)) & TG_UCHAR_MAX)
#define SYM_CROPUS(n) (((unsigned long)(n)) & TG_USHRT_MAX)
#define SYM_CROPUI(n) (((unsigned long)(n)) & TG_UINT_MAX)
#define SYM_CROPUL(n) (((unsigned long)(n)) & TG_ULONG_MAX)

/* checks if two scopes are same */
#define SYM_SAMESCP(sym, scp) ((sym)->scope == (scp) ||                                \
                               ((sym)->scope == SYM_SPARAM && (scp) == SYM_SLOCAL))

/* checks if value fits in bit-field;
   ASSUMPTION: 2sC for signed integers assumed */
#define SYM_INFIELD(v, p) ((v >= 0 && v <= (SYM_FLDMASK(p) >> 1)) ||           \
                           (v < 0 && v >= -(int)(SYM_FLDMASK(p) >> 1) - 1))


#endif    /* SYM_H */

/* end of sym.h */
