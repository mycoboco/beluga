/*
 *  null binding
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE */
#include <cbl/assert.h>    /* assert */
#include <cel/opt.h>       /* opt_t, opt_reinit, opt_parse, opt_errmsg */

#include "bnull.h"    /* common.h, dag.h, gen.h, ir.h, lex.h, op.h, sym.h */

#define S(p) ((sym_t *)(p))    /* shorthand for cast to sym_t * */


static int fprog;    /* true when progbeg() invoked */
static void *nonnull = "placeholder";


/*
 *  sets x of an address symbol
 */
static void symaddr(sym_t *p, sym_t *q, ssz_t n)
{
    assert(p);
    assert(q);
#ifdef NDEBUG
    UNUSED(q);
#endif    /* NDEBUG */
    UNUSED(n);

    assert(!p->x.name);
    assert(q->x.name);
    p->x.name = nonnull;
}


/*
 *  sets x of a global/static/constant symbol
 */
static void symgsc(sym_t *p)
{
    assert(p);

    p->x.name = nonnull;
}


/*
 *  sets x of a local
 */
static void symlocal(sym_t *p)
{
    assert(p);

    p->x.name = nonnull;
}


/*
 *  recognizes target-specific options
 */
static void option(int *pc, char **pv[], void (*oerr)(const char *, ...))
{
    static opt_t tab[] = {
        NULL,
    };

    int c;
    const void *argptr;

    assert(oerr);

    if (!opt_reinit(tab, pc, pv, &argptr))
        oerr("failed to parse options\n");

    while ((c = opt_parse()) != -1) {
        switch(c) {
            /* common case labels follow */
            case 0:    /* flag variable set; do nothing else now */
                break;
            case '?':    /* unrecognized option */
            case '-':    /* no or invalid argument given for option */
            case '+':    /* argument given to option that takes none */
            case '*':    /* ambiguous option */
                oerr(opt_errmsg(c), (const char *)argptr);
                break;
            default:
                assert(!"not all options covered -- should never reach here");
                break;
        }
    }
}


/*
 *  initializes a program
 */
static void progbeg(FILE *outfp)
{
    assert(outfp);
#ifdef NDEBUG
    UNUSED(outfp);
#endif    /* NDEBUG */

    fprog = 1;
}


/*
 *  finalizes a program
 */
static void progend(void)
{
    assert(fprog);
}


/*
 *  starts a block;
 *  overrides gen_blkbeg
 */
static void blockbeg(gen_env_t *env)
{
    UNUSED(env);
}


/*
 *  ends a block;
 *  overrides gen_blkend
 */
static void blockend(const gen_env_t *env)
{
    UNUSED(env);
}


/*
 *  defines a global
 */
static void defglobal(sym_t *p)
{
    assert(p);
#ifdef NDEBUG
    UNUSED(p);
#endif    /* NDEBUG */

    assert(p->u.seg);
    assert(!p->x.f.imported);
    assert(p->sclass == LEX_STATIC || p->x.f.exported);
}


/*
 *  completes a global array
 */
static void cmpglobal(sym_t *p)
{
    assert(p);
#ifdef NDEBUG
    UNUSED(p);
#endif    /* NDEBUG */

    assert(p->u.seg);
    assert(!p->x.f.imported);
    assert(p->sclass == LEX_STATIC || p->x.f.exported);
    assert(TY_ISARRAY(p->type));
    assert(p->type->size > 0);
}


/*
 *  provides an initializer for an address symbol
 */
static void initaddr(sym_t *p)
{
    assert(p);
#ifdef NDEBUG
    UNUSED(p);
#endif    /* NDEBUG */
}


/*
 *  provides a constant initializer
 */
static void initconst(int ty, sym_val_t v)
{
    UNUSED(ty);
    UNUSED(v);
}


/*
 *  provides a string initializer
 */
static void initstr(ssz_t n, const char *s)
{
    assert(n > 0);
    assert(s);
#ifdef NDEBUG
    UNUSED(n);
    UNUSED(s);
#endif    /* NDEBUG */
}


/*
 *  provides a zero-padded initializer
 */
static void initspace(ssz_t n)
{
    assert(n > 0);
#ifdef NDEBUG
    UNUSED(n);
#endif    /* NDEBUG */
}


/*
 *  exports a symbol
 */
static void export(sym_t *p)
{
    assert(p);

    assert(p->x.name);
    p->x.f.exported = 1;
}


/*
 *  imports a symbol
 */
static void import(sym_t *p)
{
    assert(p);

    assert(!p->x.f.imported);
    assert(p->x.name);
    assert(!p->x.f.exported);
    p->x.f.imported = 1;
}


/*
 *  processes a function
 */
static void function(sym_t *f, void *caller[], void *callee[], int n)    /* sym_t */
{
    int i;

    assert(f);
    assert(caller);
    assert(callee);
#ifdef NDEBUG
    UNUSED(f);
    UNUSED(caller);
#endif    /* NDEBUG */
    UNUSED(n);

    assert(!f->x.f.imported);
    assert(f->sclass == LEX_STATIC || f->x.f.exported);

    for (i = 0; callee[i]; i++)
        S(callee[i])->x.name = nonnull;
}


/*
 *  changes the segment
 */
static void segment(int seg)
{
    assert(seg);
#ifdef NDEBUG
    UNUSED(seg);
#endif    /* NDEBUG */
}


/* IR interface for null binding */
ir_t ir_bnull = {
     1, 1, 0,    /* charmetric */
     2, 2, 0,    /* shortmetric */
     4, 4, 0,    /* intmetric */
     4, 4, 0,    /* longmetric */
#ifdef SUPPORT_LL
     8, 4, 0,    /* llongmetric */
#endif    /* SUPPORT_LL */
     4, 4, 0,    /* floatmetric */
     8, 8, 0,    /* doublemetric */
    16, 8, 0,    /* ldoublemetric */
     4, 4, 0,    /* ptrmetric */
     0, 4, 0,    /* structmetric */
    {
        1,    /* little_endian */
        1,    /* little_bit */
        0,    /* want_callb */
        0,    /* want_argb */
        0,    /* left_to_right */
        0     /* want_dag */
    },
    NULL,         /* out */
    symaddr,
    symgsc,
    symlocal,
    option,
    progbeg,
    progend,
    blockbeg,
    blockend,
    defglobal,
    cmpglobal,
    initaddr,
    initconst,
    initstr,
    initspace,
    export,
    import,
    function,
    NULL,         /* emit */
    gen_code,
    segment,
    {
        0,       /* fmt */
        0,       /* nreg */
        0,       /* nnt */
        NULL,    /* rule */
        NULL,    /* ntname */
        NULL,    /* rmapw */
        NULL,    /* rmaps */
        NULL,    /* prerewrite */
        NULL,    /* target */
        NULL,    /* clobber */
        NULL     /* emit */
    }
};

/* end of bnull.c */
