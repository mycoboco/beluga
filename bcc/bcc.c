/*
 *  beluga driver (bcc)
 */

#include <ctype.h>         /* isdigit */
#include <errno.h>         /* errno */
#include <limits.h>        /* CHAR_BIT */
#include <signal.h>        /* SIG*, SIG_*, signal */
#include <stdarg.h>        /* va_list, va_start, va_end */
#include <stddef.h>        /* NULL */
#include <stdio.h>         /* fprintf, vfprintf, putc, stderr, remove, FILE, fopen, fclose */
#include <stdlib.h>        /* strtol, exit, EXIT_FAILURE */
#include <string.h>        /* strtok, strchr, strcmp, strcpy, strcat, strrchr, strlen */
#include <cbl/arena.h>     /* arena_t, ARENA_NEW, ARENA_ALLOC, ARENA_DISPOSE */
#include <cbl/assert.h>    /* assert */
#include <cdsl/dlist.h>    /* dlist_t, dlist_new, dlist_length, dlist_get, dlist_addtail,
                              dlist_free */
#include <cdsl/hash.h>     /* hash_string, hash_new, hash_reset */
#include <cdsl/table.h>    /* table_t, table_new, table_put, table_get, table_free */
#include <sys/types.h>     /* pid_t */
#include <unistd.h>        /* fork, execv, getpid */
#include <sys/wait.h>      /* wait, W* */

#include "ec.h"
#include "../version.h"

#define PRGNAME  "bcc"
#define AUTHOR   "Jun Woong"
#define CONTACT  "woong.jun@gmail.com"
#define HOMEPAGE "http://code.woong.org/beluga"

#define NELEM(a) (sizeof(a)/sizeof(*(a)))    /* # of elements in array */

/* stringification */
#define str(x) #x
#define xstr(x) str(x)

#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR /
#endif    /* !DIR_SEPARATOR */

#define DSEP (xstr(DIR_SEPARATOR)[0])    /* separator for directory */

/* temporary directory */
#ifndef TMP_DIR
#define TMP_DIR "/tmp/"
#endif    /* TMP_DIR */


/* internal functions referenced forwardly */
static void escape(const char *, const char *);


/* option list type;
   see xopt.h and getb() for assigned values */
enum {
    LC  = 0,    /* options for compiler */
    LLO = 1,    /* options for linker */
    LS  = 2,    /* options for assembler */
    LI,         /* input files */
    LLI,        /* input files for linker */
    LR,         /* files to remove */
    LMAX
};

/* file extension type */
enum {
    TC,    /* .c */
    TS,    /* .s */
    TO     /* .o, .obj */
};


static const char *prgname;    /* program name */

/* option conversion table */
static struct {
    const char *gcc;                                /* gcc's option */
    struct oset {
        const char *beluga[2];                      /* beluga's options (beluga, ld) */
        void (*esc)(const char *, const char *);    /* escape function */
        struct oset *next;                          /* next option set */
    } oset;
} omap[] = {
#define dd(a, b, c)
#define tt(a)
#define xx(a, b, c, d, e, f) a,        { b,                      c,    d,    },
#define ww(a, b, c)          "W" a,    { "--won " xstr(EC_##b),  NULL, NULL, },    \
                             "Wno-" a, { "--woff " xstr(EC_##b), NULL, NULL, },
#include "xopt.h"
};

static arena_t *strg;                     /* arena */
static table_t *otab;                     /* option conversion table */
static dlist_t *ls[LMAX];                 /* option lists */
static int flagE, flagc, flagS, flagv;    /* driver flags */
static const char *outfile;               /* output file */
static int ecnt;                          /* # of errors occurred */
static char buf[64];                      /* common buffer to handle options */

/* predefined command for beluga */
static const char *beluga[] = {
#include "host/beluga.h"
    NULL
};

/* predefined command for as */
static const char *as[] = {
#include "host/as.h"
    NULL
};

/* predefined command for ld */
static const char *ld[] = {
#include "host/ld.h"
    NULL
};


/*
 *  removes temporary files
 */
static void rm(void)
{
    int i, n;

    if (!ls[LR])
        return;

    n = dlist_length(ls[LR]);
    if (flagv && n > 0)
        fputs("rm", stderr);
    for (i = 0; i < n; i++) {
        const char *f = dlist_get(ls[LR], i);
        if (flagv)
            fprintf(stderr, " %s", f);
        if (flagv > 1)
            continue;
        remove(f);
    }
    if (flagv && n > 0)
        putc('\n', stderr);
}


/*
 *  handles a signal
 */
static void handler(int sig)
{
    ((void)sig);    /* unused */

    rm();
    exit(EXIT_FAILURE);
}


/*
 *  establishes signal handlers
 */
static void initsig(void)
{
    int i;
    int s[] = { SIGTERM, SIGINT,
#ifdef SIGHUP
        SIGHUP
#endif    /* SIGHUP */
    };

    for (i = 0; i < NELEM(s); i++)
        if (signal(s[i], SIG_IGN) != SIG_IGN)
            signal(s[i], handler);
}


/*
 *  initializes the option mapping table
 */
static void inittab(void)
{
    int i, n;
    struct oset *p;
    const char *s;

    otab = table_new(NELEM(omap), NULL, NULL);

    for (i = 0; i < NELEM(omap); i++) {
        if (omap[i].gcc[1] != '?' && (s = strchr(omap[i].gcc, '?')) != NULL) {
            n = s - omap[i].gcc;
            assert(n > 0 && s[-1] == ' ' && s[1] == '\0');
            assert(n < sizeof(buf));
            strncpy(buf, omap[i].gcc, --n);
            buf[n] = '=', buf[n+1] = '\0';
            omap[i].gcc = buf;    /* invalidates omap */
        }
        p = table_put(otab, hash_string(omap[i].gcc), &omap[i].oset);
        if (p)
            omap[i].oset.next = p;
    }
}


/*
 *  initializes option lists
 */
static void initlist(void)
{
    int i;

    for (i = 0; i < NELEM(ls); i++)
       ls[i] = dlist_new();
}


/*
 *  cleans up
 */
static void clean(void)
{
    int i;

    rm();
    for (i = 0; i < LMAX; i++)
        if (ls[i])
            dlist_free(&ls[i]);
    if (otab)
        table_free(&otab);
    if (strg)
        ARENA_DISPOSE(&strg);
    hash_reset();
}


/*
 *  issues a diagnostic and terminates if necessary
 */
static void error(int fatal, const char *fmt, ...)
{
    va_list ap;

    assert(fmt);

    va_start(ap, fmt);
    fprintf(stderr, "%s: ", prgname);
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);

    if (fatal) {
        clean();
        exit(EXIT_FAILURE);
    }
}


/*
 *  adds comma-separated values to an option list
 */
static void addcsv(dlist_t *l, char *v)
{
    char *p;

    assert(l);
    assert(v);

    for (p = v; (p = strtok(p, ",")) != NULL; p = NULL)
        dlist_addtail(l, p);
}


/*
 *  prints the version and terminates
 */
static void version(void)
{
    const char *verstr =
        "bcc: a standard C compiler " VERSION "\n"
        "This is free software; see the LICENSE file for more information. There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "\n"
        "Written by " AUTHOR " based on lcc developed by Chris Fraser and David Hanson.";

    puts(verstr);

    exit(0);
}


#define M 79    /* line length */
#define I 2     /* indent space */
#define L 17    /* length for options */
#define G 2     /* space between option and message */

/*
 *  finds a space from a substring
 */
static const char *rspc(const char *s, const char *e)
{
    while (e > s)
        if (*--e == ' ')
            return e;

    assert(!"too long message without spaces");
    return 0;
}


/*
 *  prints space
 */
static void space(int n)
{
    while (n-- > 0)
        putc(' ', stdout);
}


/*
 *  prints a line with wrapping
 */
static void line(const char *p, int pre, int n, int i)
{
    const char *q;
    int spc = pre - n;

    for (q = p; *q; p=q+1, spc=pre+i) {
        space(spc);
        q = ((n = strlen(p)) > M-pre)? rspc(p, p+M-pre): p+n;
        assert(q);
        printf("%.*s", q-p, p);
        putc('\n', stdout);
    }
}


/*
 *  displays the help message and terminates
 */
static void help(void)
{
    static struct {
        const char *option;
        const char *value;
        const char *help;
    } opts[] = {
#define dd(a, b, c)          a,     b,    c,
#define tt(a)                NULL,  NULL, a,
#define xx(a, b, c, d, e, f) a,     e,    f,
#define ww(a, b, c)          "W" a, NULL, "turn on warnings for " c,
#include "xopt.h"
    };

    const char *p;
    int i, n, v, m;

    printf("Usage: %s [OPTION]... <FILE>...\n\n", prgname);

    for (i = 0; i < NELEM(opts); i++) {
        v = m = 0;
        if (!opts[i].option) {
            putc('\n', stdout);
            line(opts[i].help, 0, 0, 0);
            continue;
        }
        space(I);
        if ((p = strchr(opts[i].option, '?')) != NULL) {    /* -x? or -long ? */
            assert(p[1] == '\0');
            printf("-%.*s", p-opts[i].option, opts[i].option);
            m = -1;
        } else
            printf("-%s", opts[i].option);
        if (opts[i].value) {
            fputs(opts[i].value, stdout);
            v = strlen(opts[i].value);
        }
        if ((n = strlen(opts[i].option)+m+v+1) > L)
            n = -2;
        if (n < 0)
            putc('\n', stdout);
        line(opts[i].help, I+L+G, n+2, 2);
    }

    puts("\nFor bug reporting instructions, please see:\n"
         "<" HOMEPAGE ">.");

    exit(0);
}

#undef M
#undef I
#undef L
#undef G


/*
 *  handles driver options
 */
static int dopt(char *argv[])
{
    char *arg;

    assert(argv);
    assert(argv[0] && *argv[0] == '-');

    arg = argv[0] + 1;    /* skips - */

    switch(*arg) {
        case 'E':    /* -E */
            if (arg[1] == '\0')
                flagE = 1;
            return 0;    /* passes -E to beluga */
        case 'c':    /* -c */
            if (arg[1] == '\0')
                flagc = 1;
            break;
        case 'S':    /* -S */
            if (arg[1] == '\0')
                flagS = 1;
            break;
        case 'l':    /* -l */
            if (arg[1] != '\0') {
                assert(arg[-1] == '-');
                dlist_addtail(ls[LI], arg-1);
            }
            break;
        case 'o':    /* -o */
            if (argv[1]) {
                outfile = argv[1];
                return 2;
            } else
                error(1, "file name must be given to `-o'");
            break;
        case 'v':    /* -v */
            if (arg[1] == '\0')
                flagv++;
            break;
        case '-':
            if (strcmp(arg+1, "version") == 0)    /* --version */
                version();
            if (strcmp(arg+1, "help") == 0)    /* --help */
                help();
            return 0;
        case 'W':    /* -Wl,... */
            {
                const char *s = "pcal";
                const int t[] = { LC, LC, LS, LLO };

                if (arg[2] == ',' && arg[3] != '\0') {
                    const char *p = strchr(s, arg[1]);
                    if (!p)
                        return 0;
                    addcsv(ls[t[p-s]], arg+3);
                    break;
                }
            }
            /* no break */
        default:
            return 0;
    }

    return 1;
}


/*
 *  extracts an option from argv[]
 */
static const char *extract(const char **parg, const char *next, const char **pv)
{
    const char *h, *arg;

    assert(parg);
    assert(pv);

    arg = *parg;
    assert(arg);

    if ((h = strchr(arg, '=')) != NULL) {
        *pv = h + 1;
        return (*parg = hash_new(arg, h-arg+1));
    } else {
        int n = strlen(arg);
        if (n+2 <= sizeof(buf)) {    /* +2 for = and NUL */
            strcpy(buf, arg);
            buf[n] = '=';
            buf[n+1] = '\0';
            h = hash_string(buf);
            if (table_get(otab, h)) {
                *pv = (next)? next: "";
                return h;
            }
        }
    }

    *pv = NULL;
    return hash_string(arg);
}


/*
 *  adds beluga's option to a list
 */
static void addopt(dlist_t *l, const char *arg, const char *opt, const char *v)
{
    const char *s, *p;

    assert(l);
    assert(arg);
    assert(opt);

    for (s = opt; *s != '\0'; s++)
        switch(*s) {
            case '+':    /* unsigned integer */
                assert(v);
                errno = 0;
                if (!isdigit(*v) || (strtol(v, (void *)&p, 10), *p != '\0' || errno))
                    error(1, "argument to `-%s' must be a non-negative integer in proper range",
                          arg);
                /* no break */
            case '$':    /* string */
                assert(v);
                dlist_addtail(l, (void *)v);
                opt = s + 1;
                break;
            case ' ':
                if (s > opt)
                    dlist_addtail(l, (void *)hash_new(opt, s-opt));
                opt = s + 1;
                break;
            case '\\':
                if (s[1] != '\0')
                    s++;
                break;
        }

    if (*opt != '\0')
        dlist_addtail(l, (void *)opt);
}


/*
 *  gets beluga's option for argv[]
 */
static int getb(const char *arg, const char *next)
{
    int n = 1;
    struct oset *p;
    char ho[] = "x?";
    const char *h, *v;

    assert(arg);

    ho[0] = arg[0];
    h = hash_string(ho);
    if ((p = table_get(otab, h)) != NULL) {
        if (arg[1] != '\0')
            v = &arg[1];
        else if (next) {
            v = next;
            n++;
        } else
            v = "";
    } else {
        h = extract(&arg, next, &v);
        if (v == next)
            n++;
        p = table_get(otab, h);
    }
    if (p) {
        do {
            if (p->esc)
                p->esc(arg, v);
            else if (v && *v == '\0')
                error(1, "missing argument to `-%s'", arg);
            else {
                int j;
                for (j = 0; j < NELEM(p->beluga); j++)
                    if (p->beluga[j])
                        addopt(ls[j], h, p->beluga[j], v);
            }
        } while((p=p->next) != NULL);
    } else
        error(0, "ignored unsupported option `-%s'", arg);

    return n;
}


/*
 *  finds a string from a mapping table
 */
static const char *match(const char *t[], const char *v)
{
    assert(t);
    assert(v);

    for (; *t; t += 2)
        if (strcmp(*t, v) == 0)
            return *t;

    return NULL;
}


/*
 *  prints candidate arguments for an option
 */
static void candidate(const char *opt, const char *t[])
{
    int m, n;

    assert(opt);
    assert(t);
    assert(*t);

    n = 0;
    for (; *t; t += 2) {
        m = strlen(*t);
        if (n+m+((t[2])? 5: 0) < NELEM(buf)) {
            strcpy(buf+n, *t);
            n += m;
            if (t[2]) {
                strcpy(buf+n, ", ");
                n += 2;
            }
        } else
            break;
    }
    if (*t) {
        assert(n+3 < NELEM(buf));
        strcpy(buf+n, "...");
    }

    fprintf(stderr, "%s: valid arguments to `-%s' are: %s\n", prgname, opt, buf);

    clean();
    exit(EXIT_FAILURE);
}


/*
 *  handles special cases for options
 */
static void escape(const char *opt, const char *v)
{
    static const char *map[] = {
        "std=", "std"
    };

    int i;
    const char *p;

    assert(opt);
    assert(v);

    for (i = 0; i < NELEM(map); i++)
        if (strcmp(map[i], opt) == 0)
            break;

    switch(i) {
        case 0:    /* -std=, -std */
        case 1:
            {
                static const char *tv[] = { "c90",            "c90",
                                            "c89",            "c90",
                                            "iso9899:1990",   "c90",

                                            "iso9899:199409", "c95",

#if 0    /* disabled until major C99/C11 features are implemented */
                                            "c99",            "c99",
                                            "c9x",            "c99",
                                            "iso9899:1999",   "c99",
                                            "iso9899:199x",   "c99",

                                            "c11",            "c11",
                                            "c1x",            "c11",
                                            "iso9899:2011",   "c11",
#endif    /* disabled */
                                            NULL };
                if ((p = match(tv, v)) == NULL) {
                    error(0, "invalid argument to `-%s'", opt);
                    candidate(opt, tv);
                }
                dlist_addtail(ls[LC], "--std");
                dlist_addtail(ls[LC], (void *)p);
            }
            break;
        default:
            assert(!"invalid option -- should never reach here");
            break;
    }
}


/*
 *  checks if a file can be read
 */
static int exist(const char *f)
{
    FILE *fp;

    assert(f);

    if (f[0] == '-' && f[1] == '\0')
        return 1;

    fp = fopen(f, "r");
    if (!fp)
        return 0;

    fclose(fp);
    return 1;
}


/*
 *  composes a filename
 */
static const char *ncat(const char *tmp, const char *base, const char *sfx, const char *ext)
{
    char *p;

    assert(tmp);
    assert(base);
    assert(sfx);
    assert(ext);

    p = ARENA_ALLOC(strg, strlen(tmp)+strlen(base)+strlen(sfx)+strlen(ext) + 1);
    sprintf(p, "%s%s%s%s", tmp, base, sfx, ext);

    return p;
}


/*
 *  constructs a command to run
 */
static char **compose(const char *a[], dlist_t *opt, dlist_t *in, dlist_t *out)
{
    char **oa;
    int i, n, c;
    dlist_t *ls[3], *ol = dlist_new();

    assert(a);
    assert(opt);

    ls[0] = opt;
    ls[1] = in;
    ls[2] = out;

    for (; *a; a++) {
        const char *p = *a;
        if (p && p[0] != '\0') {
            if (p[0] == '$' && isdigit(p[1])) {
                c = p[1] - '1';
                assert(c >= 0 && c <= 2);
                if (ls[c]) {
                    n = dlist_length(ls[c]);
                    for (i = 0; i < n; i++)
                        dlist_addtail(ol, dlist_get(ls[c], i));
                }
            } else
                dlist_addtail(ol, (void *)p);
        }
    }

    n = dlist_length(ol);
    oa = ARENA_ALLOC(strg, sizeof(*oa) * (n+1));
    for (i = 0; i < n; i++)
        oa[i] = dlist_get(ol, i);
    oa[i] = NULL;
    dlist_free(&ol);

    return oa;
}


/*
 *  prints a command to run
 */
static void vout(char *arg[])
{
    fputs(*arg, stderr);
    while (*++arg)
        fprintf(stderr, " %s", *arg);
    putc('\n', stderr);
}


/*
 *  run a command
 */
static int run(char *arg[])
{
    int st;
    pid_t pid;

    assert(arg);

    if (flagv)
        vout(arg);
    if (flagv > 1)
        return 1;
    if ((pid = fork()) < 0)
        error(1, "failed to create process");
    else if (pid > 0) {    /* parent */
        if (wait(&st) < 0)
            error(1, "failed to manage process while running %s", arg[0]);
        if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {
            if (WIFSIGNALED(st))
                error(1, "error occurred while running %s", arg[0]);
            ecnt++;
            return 0;
        }
        return 1;
    } else {    /* child */
        execv(arg[0], arg);
        error(1, "failed to execute %s", arg[0]);
    }

    return 1;
}


/*
 *  finds a file from the linker's input list
 */
static int findlli(const char *f)
{
    int i, n;

    assert(f);

    n = dlist_length(ls[LLI]);
    for (i = 0; i < n; i++)
        if (strcmp(f, dlist_get(ls[LLI], i)) == 0)
            return 1;
    return 0;
}


/*
 *  gets pid
 */
static const char *pid(void)
{
    static char id[1 + (sizeof(unsigned long)*CHAR_BIT+2)/3 + 1];

    if (id[0] == '\0')
        sprintf(id, "-%lu", (unsigned long)getpid());

    return id;
}


/*
 *  process a file
 */
static void process(const char *f, const char *bn, int type)
{
    int ok;
    const char *outn;
    dlist_t *d1, *d2;

    assert(f);
    assert(bn);

    if (f[0] != '-' && !exist(f))
        error(1, "failed to read file: %s", f);

    switch(type) {
        case TC:
            outn = (!flagE && !flagS)? ncat(TMP_DIR, bn, pid(), ".s"):
                   (outfile)? outfile:
                   (flagE)? "-": ncat("", bn, "", ".s");
            ok = run(compose(beluga, ls[LC], (d1=dlist_list((void *)f, NULL)),
                             (d2=dlist_list((void *)outn, NULL))));
            dlist_free(&d1);
            dlist_free(&d2);
            if ((flagE || flagS) && ok)
                break;
            if (!(outn[0] == '-' && outn[1] == '\0'))
                dlist_addtail(ls[LR], (void *)outn);
            if (!ok)
                break;
            f = outn;
            /* no break */
        case TS:
            if (flagE || flagS)
                break;
            outn = (!flagc)? ncat(TMP_DIR, bn, pid(), ".o"):
                   (outfile)? outfile: ncat("", bn, "", ".o");
            ok = run(compose(as, ls[LS], (d1=dlist_list((void *)f, NULL)),
                             (d2=dlist_list((void *)outn, NULL))));
            dlist_free(&d1);
            dlist_free(&d2);
            if (flagc && ok)
                break;
            dlist_addtail(ls[LR], (void *)outn);
            if (!ok)
                break;
            f = outn;
            /* no break */
        case TO:
        case -1:
            if (flagE || flagS || flagc)
                break;
            if (!findlli(f))
                dlist_addtail(ls[LLI], (void *)f);
            break;
        default:
            assert(!"invalid file type -- should never reach here");
            break;
    }
}


/*
 *  extracts a basename from a filename
 */
static const char *base(const char *f)
{
    const char *d, *s;

    assert(f);

    d = strrchr(f, '.');
    s = strrchr(f, DSEP);

    s = (s)? s+1: f;
    if (!d || d < s || s == d || d[1] == '\0')
        return s;
    return hash_new(s, d-s);
}


/*
 *  extracts an extension from a filename
 */
static const char *ext(const char *f)
{
    const char *p;

    assert(f);

    p = strrchr(f, '.');
    return (p && p > f && p[-1] != DSEP)? p+1: "";
}


/*
 *  determines a file type from an extension
 */
static int type(const char *ext)
{
    static struct {
        const char *ext;
        int type;
    } tt[] = { "c",   TC,
               "i",   TI,
               "s",   TS,
               "o",   TO,
               "obj", TO };

    int i;

    assert(ext);

    for (i = 0; i < NELEM(tt); i++)
        if (strcmp(tt[i].ext, ext) == 0)
            return tt[i].type;

    return -1;
}


#define INPUT(p) ((p)[0] != '-' || (p)[1] == '\0')

/*
 *  main function
 */
int main(int argc, char **argv)
{
    int i, n;
    const char *p;

    prgname = (argv[0][0] == '\0')? PRGNAME:
              ((p = strrchr(argv[0], DSEP)) == NULL)? argv[0]: p+1;

    initsig();
    inittab();
    initlist();
    strg = ARENA_NEW();

    for (i = 1; i < argc; )
        if (!INPUT(argv[i])) {
            if ((n = dopt(&argv[i])) == 0)
                i += getb(argv[i]+1, argv[i+1]);
            else
                i += n;
        } else
            dlist_addtail(ls[LI], argv[i++]);

    if (outfile && (flagE || flagc || flagS) && dlist_length(ls[LI]) > 1) {
        error(0, "ignored `-o' when `-E', `-c' or `-S' given with multiple files");
        outfile = NULL;
    }

    {    /* proceeds before linking */
        n = dlist_length(ls[LI]);
        if (n == 0)
            error(1, "no input files");
        for (i = 0; i < n; i++) {
            p = dlist_get(ls[LI], i);
            if (INPUT(p))
                process(p, (p[0] == '-')? "stdin": base(p), (p[0] == '-')? TC: type(ext(p)));
            else {
                assert(p[1] == 'l');
                dlist_addtail(ls[LLI], (void *)p);
            }
        }
    }

    if (!(ecnt > 0 || flagE || flagS || flagc)) {    /* invokes linker */
        dlist_t *l;

        if (!outfile)
            outfile = "a.out";
        if (!run(compose(ld, ls[LLO], ls[LLI], (l=dlist_list((void *)outfile, NULL)))))
            ecnt++;
        dlist_free(&l);
    }

    clean();

    return (ecnt == 0)? 0: EXIT_FAILURE;
}

#undef INPUT

/* end of bcc.c */
