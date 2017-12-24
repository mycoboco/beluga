/*
 *  option (cel)
 */

#include <assert.h>    /* assert */
#include <ctype.h>     /* isprint, isspace */
#include <errno.h>     /* errno */
#include <limits.h>    /* CHAR_BIT */
#include <stddef.h>    /* NULL, size_t */
#include <stdio.h>     /* sprintf */
#include <string.h>    /* memcpy, strrchr, strlen, strchr, strncmp */
#include <stdlib.h>    /* strtol, strtoul, strtod, malloc, free, getenv */

#include "opt.h"


/* normalizes characters for space */
#define normal(c) (((c) == ' ' || (c) == '_')? '-': (c))

#define NELEM(a) (sizeof(a)/sizeof(*(a)))    /* # of elements in array */


/* option kinds */
enum {
    INVALID,     /* invalid */
    DMINUS,      /* -- */
    SHORTOPT,    /* -f... */
    LONGOPT,     /* --f... */
    OPERAND      /* operand */
};


const char *opt_ambm[];                      /* ambiguous matches */
int opt_arg_req, opt_arg_no, opt_arg_opt;    /* unique storages for OPT_ARG_ macros */


/* ordering mode in which opt_parse() works */
static enum {
    PERMUTE,           /* argument permutation performed, default */
    REQUIRE_ORDER,     /* POSIX-compliant mode */
    RETURN_IN_ORDER    /* operands act as if be option-arguments for '\001' option */
} order;

static const opt_t *opt;     /* option description table from user */
static int *pargc;           /* argc from user */
static char ***pargv;        /* copy of argv */
static const void **parg;    /* object through which option-argument passed */
static const char *nopt;     /* short-named option to see next in grouped one */
static int oprdflag;         /* set if all remaining arguments are recognized as operands */
static int oargc;            /* location to copy next operand */
static int unrecog;          /* retains unrecognized arguments when set */
static const char *name;     /* program name */
static int dsep;             /* directory separator */


/*
 *  determines the kind of a program argument
 */
static int argcheck(const char *arg)
{
    if (!arg || arg[0] == '\0')
        return INVALID;

    if (!oprdflag && arg[0] == '-') {    /* -... */
        if (arg[1] == '-') {    /* --... */
            if (arg[2] == '\0') {    /* -- */
                oprdflag = 1;
                return DMINUS;
            } else    /* --f... */
                return LONGOPT;
        } else if (arg[1] == '\0')    /* - */
            return OPERAND;
        else    /* -f... */
            return SHORTOPT;
    } else
        return OPERAND;
}


/*
 *  converts an option-argument based on a type
 */
static const void *argconv(const char *arg, int type)
{
    /* provides storage for conversion result */
    static int boolean;              /* for OPT_TYPE_BOOL */
    static long inumber;             /* for OPT_TYPE_INT */
    static unsigned long unumber;    /* for OPT_TYPE_UINT */
    static double fnumber;           /* for OPT_TYPE_REAL */

    char *endptr;

    assert(arg);

    errno = 0;
    switch(type) {
        case OPT_TYPE_BOOL:
            while (*arg != '\0' && isspace(*(unsigned char *)arg))    /* ignores leading spaces */
                arg++;
            if (*arg == 't' || *arg == 'T' || *arg == 'y' || *arg == 'Y' || *arg == '1')
                boolean = 1;
            else
                boolean = 0;
            return &boolean;
        case OPT_TYPE_INT:
            inumber = strtol(arg, &endptr, 0);
            return (*endptr != '\0' || errno)? NULL: &inumber;
        case OPT_TYPE_UINT:
            unumber = strtoul(arg, &endptr, 0);
            return (*endptr != '\0' || errno)? NULL: &unumber;
        case OPT_TYPE_REAL:
            fnumber = strtod(arg, &endptr);
            return (*endptr != '\0' || errno)? NULL: &fnumber;
        case OPT_TYPE_STR:
            return arg;
        default:
            assert(!"unknown conversion requested -- should never reach here");
            break;
    }

    return NULL;
}


/*
 *  checks consistency of an option description table
 *
 *  The requirements checked are:
 *  - if the first character of the first lopt member is '+' or '-',
 *    - the member has to be a string with the sole character and its sopt member be 0; and
 *    - if the first character of the second lopt member is ' ', the member has to be " " and its
 *      sopt member be 0;
 *  - if the first character of the first lopt member is ' ', the member has to be " " and its sopt
 *    member be 0;
 *  - the sopt member has a non-negative value;
 *  - the sopt member is not allowed to have '?', '-', '+', '*', '=', and -1;
 *  - the sopt member is allowed to have 0 only when the lopt member is not empty and a flag
 *    variable provided;
 *  - the lopt member is not allowed to contain '=';
 *  - if an option takes an option-argument, its type has to be specified; and
 *  - the table has no duplicate sopt member.
 *
 *  chckvalid() is called by opt_init() if NDEBUG is not defined.
 */
static void chckvalid(const opt_t *o)
{
    assert(o);

    if (!o->lopt)
        return;
    if (o->lopt[0] == '+' || o->lopt[0] == '-')
        assert(o->lopt[1] == '\0' && (o++)->sopt == 0);
    if (o->lopt[0] == ' ')
        assert(o->lopt[1] == '\0' && (o++)->sopt == 0);
    for (; o->lopt; o++) {
        assert(o->sopt >= 0);
        assert(o->sopt != '?' && o->sopt != '-' && o->sopt != '+' && o->sopt != '*' &&
               o->sopt != '=' && o->sopt != -1);
        assert(o->sopt != 0 || (o->lopt[0] != '\0' && o->flag && o->flag != OPT_ARG_NO &&
                                o->flag != OPT_ARG_REQ && o->flag != OPT_ARG_OPT));
        assert(strchr(o->lopt, '=') == NULL);
        assert(!(o->flag == OPT_ARG_REQ || o->flag == OPT_ARG_OPT) ||
               (o->arg == OPT_TYPE_BOOL || o->arg == OPT_TYPE_INT || o->arg == OPT_TYPE_UINT ||
                o->arg == OPT_TYPE_REAL || o->arg == OPT_TYPE_STR));
        if (o->sopt && o->sopt <= UCHAR_MAX) {
            const opt_t *p;
            for (p = o + 1; p->lopt; p++)
                assert(o->sopt != p->sopt);
        }
    }
}


/*
 *  prepares to start parsing program arguments
 */
const char *(opt_init)(const opt_t *o, int *pc, char **pv[], const void **pa, const char *n,
                       int sep)
{
    char **argv;
    const char *p = NULL;

    assert(o);
    assert(pc);
    assert(pv);
    assert(pa);
    assert(n);
    assert(sep != '\0');

    assert(!opt);
    assert(!pargc);
    assert(!pargv);
    assert(!parg);

#if !defined(NDEBUG)
    chckvalid(o);
#endif    /* !NDEBUG */

    argv = malloc((*pc+1) * sizeof(*argv));
    if (!argv)
        return NULL;
    memcpy(argv, *pv, (*pc+1) * sizeof(*argv));
    *pv = argv;

    order = PERMUTE;
    opt = o;
    pargc = pc;
    pargv = pv;    /* pargv has copy of argv */
    parg = pa;
    nopt = NULL;
    oprdflag = 0;
    oargc = 1;
    unrecog = 0;
    dsep = sep;

    if (*pargc > 0) {
        if ((*pargv)[0][0] != '\0') {    /* program name exists */
            p = strrchr((*pargv)[0], (unsigned char)sep);
            if (p)
                p++;
            else
                p = (*pargv)[0];
        } else
            p = n;    /* defaults to user-given name */
        *pargc = 0;    /* increases as opt_parse() parses options */
    } else
        *pargc = -1;    /* no program name and options available */

    if (opt->lopt) {
        if (getenv("POSIXLY_CORRECT") || opt->lopt[0] == '+') {
            order = REQUIRE_ORDER;
            if (opt->lopt[0] == '+')
                opt++;
        } else if (opt->lopt[0] == '-') {
            order = RETURN_IN_ORDER;
            opt++;
        }    /* order = PERMUTE; */
        if (opt->lopt[0] == ' ') {
            unrecog = 1;
            opt++;
        }
    }

    return (name = p);
}


/*
 *  constructs a string to contain an erroneous short-named option
 */
static const char *errsopt(int sopt)
{
    static char msg[(CHAR_BIT+3)/4+4];    /* -<??> */

    sprintf(msg, (isprint((unsigned char)sopt))? "-%c": "-<%02X>", (unsigned char)sopt);

    return msg;
}


/*
 *  constructs a string to contain an erroneous long-named option
 */
static const char *errlopt(const char *lopt)
{
    static char msg[42+1];

    const char *p;

    assert(lopt);

    for (p = lopt; *p != '\0' && *p != '=' && p-lopt < sizeof(msg)-3; p++)
        continue;

    sprintf(msg, "--%.*s%s", (int)(p-lopt), lopt, (*p != '\0' && *p != '=')? "...": "");

    return msg;
}


#define ADDTOARGV() (argv[oargc++] = argv[argc])
#define CHKNSETN()  if (n && !nopt)    /* !!n implies unrecog */    \
                        (*n='\0', n=NULL, ADDTOARGV())

/*
 *  parses program options
 *
 *  At its early development stage, opt_parse() has separate case labels for SHORTOPT and LONGOPT
 *  without fall-through. Found that two fragments of code belonging to those separate labels
 *  resemble each other very much, those two branches are merged, which is what it is now. This is
 *  a little tricky (e.g., when kind is LONGOPT, jumping into the middle of a loop is made) and
 *  brings some otherwise unnecessary checks for kind.
 */
int (opt_parse)(void)
{
    static char *n;

    int i;
    int kind;
    int retval;
    const opt_t *p;
    int argc;
    char **argv;
    const void *arg;

    assert(opt);      /* opt_init() must be called before opt_parse() */
    assert(pargc);
    assert(pargv);
    assert(parg);

    argc = *pargc;
    argv = *pargv;
    retval = 0;
    arg = NULL;
    opt_ambm[0] = NULL;

    CHKNSETN();
    if (argc < 0 || (nopt == (void *)&oargc) || argv[argc+1] == NULL) {    /* done */
        argv[oargc] = NULL;
        retval = -1;
        argc = oargc;
        nopt = (void *)&oargc;    /* prevents opt_parse() from running further */
        goto retcode;
    }

    do {
        CHKNSETN();
        switch(kind = argcheck(argv[++argc])) {
            case SHORTOPT:    /* -f... */
                if (nopt) {    /* starts at next in grouped options */
                    assert(*nopt != '\0');
                    i = nopt - argv[argc];
                    nopt = NULL;
                } else
                    i = 1;
                for (; kind != LONGOPT && argv[argc][i] != '\0'; i++) {
                    int match;
            case LONGOPT:     /* --f... */
                    match = 0;    /* cannot be merged into initialization above */
                    for (p = opt; p->lopt; p++) {
                        if (kind == SHORTOPT && argv[argc][i] == (unsigned char)p->sopt)
                            match = 1;
                        else if (kind == LONGOPT && p->lopt[0] == argv[argc][2]) {
                            char *s;
                            const opt_t *q;
                            if ((s = strchr(argv[argc]+3, '=')) != NULL)    /* --f...=... */
                                i = s - (argv[argc]+3);
                            else
                                i = strlen(argv[argc]+3);
                            q = p;
                            do {    /* checks ambiguity */
                                if (q->lopt[0] == argv[argc][2] &&
                                    strncmp(q->lopt+1, argv[argc]+3, i) == 0) {
                                    /* prefix matched */
                                    if (q->lopt[1+i] == '\0') {    /* exact match */
                                        p = q;
                                        match = 1;
                                        break;
                                    }
                                    p = q;
                                    if (match < NELEM(opt_ambm))
                                        opt_ambm[match] = q->lopt;
                                    match++;
                                }
                            } while((++q)->lopt);
                            if (!match)
                                break;    /* assigning q to p is equivalent */
                            else if (match > 1) {    /* ambiguous prefix */
                                for (; match < NELEM(opt_ambm); match++)
                                    opt_ambm[match] = NULL;
                                retval = '*';
                                arg = errlopt(argv[argc]+2);
                                goto retcode;
                            }
                            i += 2;    /* adjusts i for use below */
                            assert(argv[argc][i+1] == '\0' || argv[argc][i+1] == '=');
                        }
                        if (match) {
                            retval = p->sopt;
                            if (p->flag == OPT_ARG_REQ || p->flag == OPT_ARG_OPT) {
                                if (argv[argc][i+1] != '\0')    /* -f..., -f=..., --f...=... */
                                    arg = argconv(&argv[argc][i+1+(argv[argc][i+1]=='=')], p->arg);
                                else if (argcheck(argv[argc+1]) == OPERAND) {
                                    arg = argconv(argv[argc+1], p->arg);
                                    if (arg || p->flag == OPT_ARG_REQ)    /* extra arg consumed */
                                        argc++;
                                } else if (p->flag == OPT_ARG_OPT)
                                    arg = p->lopt;    /* arg should not be null here */
                                if (!arg) {
                                    if (p->flag == OPT_ARG_OPT && argv[argc][i+1] != '=') {
                                        /* argv[argc][i+1] != '\0' implies kind == SHORTOPT */
                                        if (argv[argc][i+1] != '\0') {
                                            nopt = &argv[argc][i+1];
                                            argc--;
                                        }
                                    } else {    /* no or invalid arg for OPT_ARG_REQ */
                                        retval = '-';
                                        arg = (kind == LONGOPT)? errlopt(p->lopt):
                                                                 errsopt(p->sopt);
                                    }
                                } else if (arg == p->lopt)
                                    arg = NULL;    /* set back it to null */
                            } else if (argv[argc][i+1] == '=') {
                                retval = '+';
                                arg = (kind == LONGOPT)? errlopt(p->lopt): errsopt(p->sopt);
                            } else if (p->flag != OPT_ARG_NO && p->flag) {    /* flag var */
                                *(p->flag) = p->arg;
                                retval = 0;
                                break;
                            } else {    /* no flag variable, so returns option */
                                assert(kind == SHORTOPT || retval != 0);
                                if (kind == SHORTOPT && argv[argc][i+1] != '\0') {
                                    nopt = &argv[argc][i+1];
                                    argc--;
                                }
                            }
                            goto retcode;
                        }
                    }
                    if (!match) {
                        if (unrecog) {
                            if (kind == SHORTOPT) {
                                if (!n)
                                    n = &argv[argc][1];
                                *n++ = argv[argc][i];
                            } else
                                ADDTOARGV();
                        } else {
                            retval = '?';
                            arg = (kind == LONGOPT)? errlopt(argv[argc]+2): errsopt(argv[argc][i]);
                            goto retcode;
                        }
                    }
                }
                break;
            case OPERAND:    /* arranges ordinary arguments for later use */
                if (order == RETURN_IN_ORDER) {
                    retval = 1;
                    arg = argv[argc];
                    goto retcode;
                } else {    /* order == PERMUTE || order == REQUIRE_ORDER */
                    if (!oprdflag && order == REQUIRE_ORDER)
                        oprdflag = 1;
                    ADDTOARGV();
                }
                break;
            case DMINUS:    /* -- */
                if (unrecog)
                    ADDTOARGV();
                break;
            default:
                assert(!"invalid type of arguments -- should never reach here");
                break;
        }
    } while(argv[argc+1]);

    retcode:
        *parg = arg;
        *pargc = argc;
        return retval;
}

#undef ADDTOARGV
#undef CHKNSETN


/*
 *  compares strings for argument comparison
 */
static int striscmp(const char *s, const char *t, int flag)
{
    unsigned char c, d;

    assert(s);
    assert(t);

    do {
        c = *(unsigned char *)s++;
        d = *(unsigned char *)t++;
        if (flag & OPT_CMP_NORMSPC)
            c = normal(c), d = normal(d);
        if (flag & OPT_CMP_CASEIN)
            c = tolower(c), d = tolower(d);
    } while(c == d && c != '\0');

    return (c != '\0' || d != '\0');
}


/*
 *  compares a string argument to a set of strings
 */
int (opt_val)(opt_val_t *tab, const char *s, int flag)
{
    assert(tab);
    assert(s);

    for (; tab->str; tab++)
        if (striscmp(tab->str, s, flag) == 0)
            break;

    return tab->val;
}


/*
 *  aborts parsing options
 */
void (opt_abort)(void)
{
    int argc;
    char **argv;

    assert(opt);      /* opt_init() must be called before opt_abort() */
    assert(pargc);
    assert(pargv);
    assert(parg);

    argc = *pargc;
    argv = *pargv;

    while(argv[argc+1])
        argv[oargc++] = argv[++argc];
    argv[oargc] = NULL;

    *pargc = oargc;
    nopt = (void *)&oargc;    /* prevents opt_parse() from running further */
}


/*
 *  constructs a string to show ambiguous matches
 */
const char *(opt_ambmstr)(void)
{
    static char buf[64];

    int i;
    size_t m, n;

    if (!opt_ambm[0])
        return "";

    n = 0;
    for (i = 0; i < NELEM(opt_ambm)-1 && opt_ambm[i]; i++) {
        m = strlen(opt_ambm[i]);
        if (n+m+((opt_ambm[i+1])? 5: 0) < NELEM(buf)) {
            strcpy(buf+n, opt_ambm[i]);
            n += m;
            if (opt_ambm[i+1]) {
                strcpy(buf+n, ", ");
                n += 2;
            }
        } else
            break;
    }
    if (opt_ambm[i]) {
        assert(n+3 < NELEM(buf));
        strcpy(buf+n, "...");
    }

    return buf;
}


/*
 *  returns a diagnostic format string for an error code
 */
const char *(opt_errmsg)(int c)
{
    const char *p, *tab = "?-+*";
    const char *msg[] = {
        "unknown option '%s'\n",
        "no or invalid argument given for '%s'\n",
        "option '%s' takes no argument\n",
        "ambiguous option '%s' (%s)\n",
        "not all options covered\n"
    };

    if ((p = strchr(tab, c)) == NULL) {
        assert(!"not all options covered -- should never reach here");
        p = tab + sizeof(msg)/sizeof(*msg)-1;
    }

    assert((p-tab) >= 0 && (p-tab) < sizeof(msg)/sizeof(*msg));

    return msg[p - tab];
}


/*
 *  cleans up any storage used and disables the internal state
 */
void (opt_free)(void)
{
    if (pargv)
        free(*pargv);

    opt = NULL;
    pargc = NULL;
    pargv = NULL;
    parg = NULL;
}


/*
 *  prepares to rescan program arguments
 */
const char *(opt_reinit)(const opt_t *o, int *pc, char **pv[], const void **pa)
{
    char **p;

    assert(o);
    assert(pc);
    assert(pv);
    assert(pa);

    assert(name);
    assert(dsep != '\0');

    assert(opt);      /* opt_init() must be called before opt_abort() */
    assert(pargc);
    assert(pargv);
    assert(parg);

    if ((p = malloc((*pc+1)*sizeof(*p))) == NULL)
        return NULL;
    memcpy(p, *pv, sizeof(*p)*(*pc+1));
    opt_free();    /* frees *pv */
    *pv = p;
    name = opt_init(o, pc, pv, pa, name, dsep);
    free(p);

    return name;
}


#if 0    /* example code */
#include <assert.h>    /* assert */
#include <stddef.h>    /* NULL */
#include <stdio.h>     /* puts, putchar, printf, fprintf, stderr */
#include <stdlib.h>    /* EXIT_FAILURE */
#include <limits.h>    /* UCHAR_MAX */

#include <cel/opt.h>    /* opt_t, opt_init, opt_parse, opt_errmsg, opt_free, OPT_ARG_*,
                           OPT_TYPE_* */

#define PRGNAME "opt-test"

struct {
    const char *prgname;    /* program name */
    int verbose;            /* set by "--verbose" and unset by "--brief" */
    int xtra;               /* set by "--xtra" */
} option;

int main(int argc, char *argv[])
{
    static opt_t tab[] = {
        " ",        0,           OPT_ARG_NO,        OPT_TYPE_NO,
        "verbose",  0,           &(option.verbose), 1,
        "brief",    0,           &(option.verbose), 0,
        "add",      'a',         OPT_ARG_NO,        OPT_TYPE_NO,
        "append",   'b',         OPT_ARG_NO,        OPT_TYPE_NO,
        "create",   'c',         OPT_ARG_REQ,       OPT_TYPE_STR,
        "delete",   'd',         OPT_ARG_REQ,       OPT_TYPE_STR,
        "file",     'f',         OPT_ARG_REQ,       OPT_TYPE_STR,
        "",         'h',         OPT_ARG_NO,        OPT_TYPE_NO,
        "integer",  'i',         OPT_ARG_OPT,       OPT_TYPE_INT,
        "number",   'n',         OPT_ARG_OPT,       OPT_TYPE_REAL,
        "",         'o',         OPT_ARG_NO,        OPT_TYPE_NO,
        "connect",  UCHAR_MAX+1, OPT_ARG_REQ,       OPT_TYPE_STR,
        "html",     UCHAR_MAX+2, OPT_ARG_NO,        OPT_TYPE_NO,
        "help",     UCHAR_MAX+3, OPT_ARG_NO,        OPT_TYPE_NO,
        "helpmore", UCHAR_MAX+4, OPT_ARG_NO,        OPT_TYPE_NO,
        "bool",     UCHAR_MAX+5, OPT_ARG_REQ,       OPT_TYPE_BOOL,
        "stop",     UCHAR_MAX+6, OPT_ARG_NO,        OPT_TYPE_NO,
        NULL,    /* must end with NULL */
    }, tab2[] = {
        "xtra", 'x', &(option.xtra), 1,
        "",     'y', OPT_ARG_NO,     OPT_TYPE_NO,
        NULL,    /* must end with NULL */
    };

    int i;
    int c;
    int connect = 0;
    const void *argptr;

    /* initial scan */
    option.prgname = opt_init(tab, &argc, &argv, &argptr, PRGNAME, '/');
    if (!option.prgname) {
        opt_free();
        fprintf(stderr, "%s: failed to parse options\n", PRGNAME);
        return EXIT_FAILURE;
    }
    printf("Program name: %s\n", option.prgname);
    while ((c = opt_parse()) != -1) {
        switch(c) {
            case 'a':
                printf("%s: option -a given\n", option.prgname);
                break;
            case 'b':
                printf("%s: option -b given\n", option.prgname);
                break;
            case 'c':
            case 'd':
            case 'f':
                printf("%s: option -%c given with value '%s'\n", option.prgname, c,
                       (const char *)argptr);
                break;
            case 'h':
                printf("%s: option -h given\n", option.prgname);
                break;
            case 'i':
                printf("%s: option -i given", option.prgname);
                if (argptr)
                    printf(" with value '%ld'\n", *(const long *)argptr);
                else    /* optional */
                    putchar('\n');
                break;
            case 'n':
                printf("%s: option -n given", option.prgname);
                if (argptr)
                    printf(" with value '%f'\n", *(const double *)argptr);
                else    /* optional */
                    putchar('\n');
                break;
            case 'o':
                printf("%s: option -o given\n", option.prgname);
                break;
            case UCHAR_MAX+1:
                {
                    opt_val_t t[] = {
                        "stdin",   0, "standard input",  0,
                        "stdout",  1, "standard output", 1,
                        "stderr",  2, "standard error",  2,
                        NULL,     -1
                    };
                    connect = opt_val(t, argptr, OPT_CMP_CASEIN | OPT_CMP_NORMSPC);
                    if (connect == -1) {
                        printf("%s: `stdin', `stdout' or `stderr' must be given for --connect\n",
                               option.prgname);
                        opt_free();
                        return EXIT_FAILURE;
                    }
                }
                break;
            case UCHAR_MAX+2:
                printf("%s: option --html given\n", option.prgname);
                break;
            case UCHAR_MAX+3:
                printf("%s: help requested\n", option.prgname);
                opt_free();
                return 0;
            case UCHAR_MAX+4:
                printf("%s: more help requested\n", option.prgname);
                opt_free();
                return 0;
            case UCHAR_MAX+5:
                printf("%s: option --boolean given with %s\n", option.prgname,
                       (*(const int *)argptr)? "true": "false");
                break;
            case UCHAR_MAX+6:
                opt_abort();
                break;

            /* common case labels follow */
            case 0:    /* flag variable set; do nothing else now */
                break;
            case '?':    /* unrecognized option */
            case '-':    /* no or invalid argument given for option */
            case '+':    /* argument given to option that takes none */
            case '*':    /* ambiguous option */
                fprintf(stderr, "%s: ", option.prgname);
                fprintf(stderr, opt_errmsg(c), (const char *)argptr, opt_ambmstr());
                opt_free();
                return EXIT_FAILURE;
            default:
                assert(!"not all options covered -- should never reach here");
                break;
        }
    }

    if (option.verbose)
        puts("verbose flag is set");
    printf("connect option is set to %d\n", connect);

    if (argc > 1) {
        printf("non-option and unrecognized ARGV-arguments:");
        for (i = 1; i < argc; i++)
            printf(" %s", argv[i]);
        putchar('\n');
    }

    /* rescan */
    if (!opt_reinit(tab2, &argc, &argv, &argptr)) {
        opt_free();
        fprintf(stderr, "%s: failed to parse options\n", PRGNAME);
        return EXIT_FAILURE;
    }

    while ((c = opt_parse()) != -1) {
        switch(c) {
            case 'y':
                printf("%s: option -y given\n", option.prgname);
                break;

            /* common case labels follow */
            case 0:    /* flag variable set; do nothing else now */
                break;
            case '?':    /* unrecognized option */
            case '-':    /* no or invalid argument given for option */
            case '+':    /* argument given to option that takes none */
            case '*':    /* ambiguous option */
                fprintf(stderr, "%s: ", option.prgname);
                fprintf(stderr, opt_errmsg(c), (const char *)argptr, opt_ambmstr());
                opt_free();
                return EXIT_FAILURE;
            default:
                assert(!"not all options covered -- should never reach here");
                break;
        }
    }

    if (option.xtra)
        puts("xtra flag is set");

    if (argc > 1) {
        printf("non-option ARGV-arguments:");
        for (i = 1; i < argc; i++)
            printf(" %s", argv[i]);
        putchar('\n');
    }

    opt_free();
    return 0;
}
#endif    /* disabled */

/* end of opt.c */
