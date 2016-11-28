/*
 *  compiler lexical analyzer
 */

#include <ctype.h>         /* isdigit, isxdigit, tolower */
#include <errno.h>         /* errno, ERANGE */
#include <float.h>         /* LDBL_MAX (when !defined(HUGE_VALL)) */
#include <limits.h>        /* CHAR_BIT */
#include <math.h>          /* HUGE_VALL */
#include <stddef.h>        /* NULL */
#include <stdlib.h>        /* strtold */
#include <string.h>        /* strchr, memcpy, memset */
#include <cbl/arena.h>     /* arena_t, ARENA_ALLOC */
#include <cbl/assert.h>    /* assert */
#include <cbl/memory.h>    /* MEM_RESIZE */
#include <cdsl/hash.h>     /* hash_string */
#ifdef HAVE_ICONV
#include <iconv.h>         /* iconv_t */
#endif    /* HAVE_ICONV */

#include "alist.h"
#include "err.h"
#include "lex.h"
#include "lmap.h"
#include "lst.h"
#include "main.h"
#include "proc.h"
#include "strg.h"
#include "sym.h"

/* ASSUMPTION: HUGE_VALL is greater than LDBL_MAX */
#ifdef HUGE_VALL
#define OVF(ld) ((ld) == HUGE_VALL)
#else    /* !HUGE_VALL */
#define OVF(ld) ((ld) > LDBL_MAX)
#endif    /* HUGE_VALL */


/* declares strtold() when no declaration is visible */
#ifndef HUGE_VALL
long double strtold(const char *, char **);
#endif    /* !HUGE_VALL */


int clx_tc;               /* token code of current token */
const char *clx_tok;      /* token spelling of current token */
sym_t *clx_sym;           /* symbol table entry for current token */
const lmap_t *clx_pos;    /* locus of current token */

/* token names */
const char * const clx_name[] = {
#define xx(a, b, c, d, e, f, g, h) g,
#define kk(a, b, c, d, e, f, g, h) g,
#define yy(a, b, c, d, e, f, g, h) g,
#include "xtoken.h"
};

/* token kinds */
const char clx_kind[] = {
#define xx(a, b, c, d, e, f, g, h) f,
#define kk(a, b, c, d, e, f, g, h) f,
#define yy(a, b, c, d, e, f, g, h) f,
#include "xtoken.h"
};


/* keyword-code table */
static struct tab {
    const char *key;     /* keyword */
    int code;            /* token code */
    struct tab *link;    /* hash chain */
} *tab[64];    /* > 32 keywords from C90 */

static int endian = 1;    /* for LITTLE from common.h */
static sym_t tval;        /* symbol value for current token */


/*
 *  initializes a keyword-code table
 */
void (clx_init)(void)
{
    static struct {
        const char *name;
        int code;
    } map[] = {
#define xx(a, b, c, d, e, f, g, h)
#define kk(a, b, c, d, e, f, g, h) g, LEX_##a,
#define yy(a, b, c, d, e, f, g, h)
#include "xtoken.h"
    };

    int i;
    unsigned h;
    struct tab *p;

    for (i = 0; i < NELEM(map); i++) {
        map[i].name = hash_string(map[i].name);
        h = hashkey(map[i].name, NELEM(tab));
        p = ARENA_ALLOC(strg_perm, sizeof(*p));
        p->key = map[i].name;
        p->code = map[i].code;
        p->link = tab[h];
        tab[h] = p;
    }

    proc_prep();
}


/*
 *  recognizes keywords and identifiers
 */
static int id(lex_t *t)
{
    unsigned h;
    const struct tab *p;

    clx_tok = hash_string(LEX_SPELL(t));
    h = hashkey(clx_tok, NELEM(tab));
    for (p = tab[h]; p; p = p->link)
        if (p->key == clx_tok)
            return p->code;

    return LEX_ID;
}


/*
 *  recognizes character constants;
 *  ASSUMPTION: char on the host has the same number of bits as on the target;
 *  ASSUMPTION: signed integers are compatible with unsigned ones on the host
 */
ux_t (clx_ccon)(lex_t *t, int *w)
{
    int cbyte;
    sz_t len = 0;
    const char *ss, *s, *e;
    ux_t lim, c;
#ifdef HAVE_ICONV
    iconv_t *cd;
#endif    /* HAVE_ICONV */

    assert(t);
    assert(w);
    assert(BUFUNIT > 2);
    assert(ty_wuchartype);    /* ensures types initialized */
    assert(UX_MAX >= TG_UCHAR_MAX);
    assert(UX_MAX >= TG_WUCHAR_MAX);
    assert(ir_cur);

    ss = s = LEX_SPELL(t);
    if (*s == 'L')
        *w = 1, s++;
    e = ++s;    /* skips ' */
    clx_pos = t->pos;

    if (*w) {
        cbyte = ty_wchartype->size;
        assert(cbyte <= BUFUNIT);
        lim = TG_WUCHAR_MAX;
#ifdef HAVE_ICONV
        cd = main_ntow;
#endif    /* HAVE_ICONV */
    } else {
        cbyte = 1;
        lim = TG_UCHAR_MAX;
#ifdef HAVE_ICONV
        cd = main_ntoe;
#endif    /* HAVE_ICONV */
    }

    switch(*e) {
        case '\'':    /* empty; diagnosed elsewhere */
        case '\0':    /* unclosed; diagnosed elsewhere */
            return 0;
        case '\\':    /* escape sequences */
            assert(sizeof(c) >= cbyte);
            c = lex_bs(t, ss, &e, lim, "character constant");
#if HAVE_ICONV
            if (cd && !(s[1] == 'x' || (s[1] >= '0' && s[1] <= '7'))) {
                char x = c;
                ICONV_DECL(&x, 1);
                assert(x == c);
                obuf = strg_sbuf, obufv = strg_sbuf;
                olen = strg_slen, olenv = strg_slen;
                ICONV_DO(cd, 1, {});
                strg_sbuf = obuf, strg_slen = olen;    /* for later reuse */
                len = strg_slen - len - olenv;
            } else {
#else    /* !HAVE_ICONV */
            {
#endif    /* HAVE_ICONV */
                if (*w)
                    memcpy(strg_sbuf, (char *)&c + ((LITTLE)? 0: sizeof(c)-cbyte), cbyte);
                else
                    strg_sbuf[0] = c;
                len = cbyte;
            }
            break;
        default:    /* ordinary chars */
#ifdef HAVE_ICONV
            if (cd) {
                do {
                    e++;
                } while(!FIRSTUTF8(*e));
                {
                    ICONV_DECL((char *)s, e - s);
                    obuf = strg_sbuf, obufv = strg_sbuf;
                    olen = strg_slen, olenv = strg_slen;
                    ICONV_DO(cd, 1, {});
                    strg_sbuf = obuf, strg_slen = olen;    /* for later reuse */
                    len = strg_slen - len - olenv;
                }
            } else {
#else    /* !HAVE_ICONV */
            {
#endif    /* HAVE_ICONV */
                assert(TG_CHAR_BIT == CHAR_BIT);
                if (*w) {
                    strg_sbuf[(LITTLE)? 0: cbyte-1] = *e;
                    memset(strg_sbuf + ((LITTLE)? 1: 0), 0, cbyte-1);
                } else
                    strg_sbuf[0] = *e;
                e++;
                len = cbyte;
            }
            break;
    }

    if (len != cbyte) {
        err_dpos(clx_pos, (*w)? ERR_CONST_WIDENOTFIT: ERR_CONST_MBNOTFIT);
        return 0;
    } else if (*e != '\'' && *e != '\0') {
        for (s = e; *e != '\0' && *e != '\''; e++)
            continue;
        err_dpos((FIRSTUTF8(*s))? lmap_spell(t, ss, s, e): clx_pos, ERR_CONST_LARGECHAR);
    }

    c = 0;
    memcpy(&c, strg_sbuf + ((LITTLE)? 0: sizeof(c)-cbyte), cbyte);
    if (*w) {
        switch(main_opt()->wchart) {
            case 0:    /* long */
                c = SYM_CROPSL(c);
                break;
            case 1:    /* ushort */
                c = SYM_CROPUS(c);
                break;
            case 2:    /* int */
                c = SYM_CROPSI(c);
                break;
        }
    } else    /* int from char */
        c = (main_opt()->uchar)? SYM_CROPUC(c): SYM_CROPSC(c);

    return c;
}


/*
 *  recognizes string literals;
 *  ASSUMPTION: char on the host has the same number of bits as on the target
 */
static sz_t scon(lex_t *t, int *w)
{
    int cbyte;
    lex_t *n;
    sz_t clen = 0, len = 0;
    const char *ss, *s, *e;
    ux_t lim;
#ifdef HAVE_ICONV
    iconv_t *cd;
#endif

    assert(t);
    assert(w);
    assert(BUFUNIT > 2);
    assert(ty_wuchartype);    /* ensures types initialized */
    assert(UX_MAX >= TG_UCHAR_MAX);
    assert(UX_MAX >= TG_WUCHAR_MAX);
    assert(ir_cur);

    ss = s = LEX_SPELL(t);
    if (*s == 'L')
        *w = 1, s++;
    e = ++s;    /* skips " */
    while ((n = lst_peekns())->id == LEX_SCON) {
        if (*n->spell == 'L') {
            if (!*w) {    /* mb + wide = wide */
                err_dmpos(n->pos, ERR_CONST_MBWIDE, t->pos, NULL);
                err_dmpos(n->pos, ERR_CONST_MBWIDESTD, t->pos, NULL);
                *w = 2;    /* silences warnings */
            }
        } else if (*w == 1) {    /* wide + mb = wide */
            err_dmpos(n->pos, ERR_CONST_MBWIDE, t->pos, NULL);
            err_dmpos(n->pos, ERR_CONST_MBWIDESTD, t->pos, NULL);
            *w = 2;    /* silences warnings */
        }
        while ((t = lst_append(t, lst_next()))->id != LEX_SCON)
            continue;
    }
    clx_pos = lmap_range(t->next->pos, t->pos);

    if (*w) {
        cbyte = ty_wchartype->size;
        assert(cbyte <= BUFUNIT);
        lim = TG_WUCHAR_MAX;
#ifdef HAVE_ICONV
        cd = main_ntow;
#endif    /* HAVE_ICONV */
    } else {
        cbyte = 1;
        lim = TG_UCHAR_MAX;
#ifdef HAVE_ICONV
        cd = main_ntoe;
#endif    /* HAVE_ICONV */
    }

    n = t->next;
    while (1) {
        while (1) {
            while (*e != '\\' && *e != '"' && *e != '\0')
                e++;
            if (e > s) {    /* ordinary chars */
#ifdef HAVE_ICONV
                if (cd) {
                    ICONV_DECL((char *)s, e - s);
                    obuf = strg_sbuf, obufv = strg_sbuf + len;
                    olen = strg_slen, olenv = strg_slen - len;
                    ICONV_INIT(cd);
                    ICONV_DO(cd, 0, {});
                    strg_sbuf = obuf, strg_slen = olen;    /* for later reuse */
                    len += (strg_slen - len - olenv);
                } else {
#else    /* !HAVE_ICONV */
                {
#endif    /* HAVE_ICONV */
                    sz_t d = e - s;
                    assert(TG_CHAR_BIT == CHAR_BIT);
                    while (len + (d*cbyte) > strg_slen)    /* rarely iterates */
                        MEM_RESIZE(strg_sbuf, strg_slen += BUFUNIT);
                    if (*w) {
                        while (s < e) {
                            strg_sbuf[len + ((ir_cur->f.little_endian)? 0: cbyte-1)] = *s++;
                            memset(strg_sbuf + len + ((ir_cur->f.little_endian)? 1: 0), 0,
                                   cbyte-1);
                            len += cbyte;
                        }
                    } else {
                        memcpy(&strg_sbuf[len], s, d);
                        len += d;
                    }
                }
                for (; s < e; s++)
                    if (FIRSTUTF8(*s))
                        clen++;
            }
            if (*e == '\\') {    /* escape sequences */
                ux_t c;
                assert(sizeof(c) >= cbyte);
                c = lex_bs(n, ss, &e, lim, "string literal");
#if HAVE_ICONV
                if (cd) {    /* inserts initial seq before every esc seq */
                    ICONV_DECL(NULL, 0);
                    UNUSED(ilenv);
                    UNUSED(ibufv);
                    obuf = strg_sbuf, obufv = strg_sbuf + len;
                    olen = strg_slen, olenv = strg_slen - len;
                    ICONV_INIT(cd);
                    strg_sbuf = obuf, strg_slen = olen;    /* for later reuse */
                    len += (strg_slen - len - olenv);
                }
                if (cd && !(s[1] == 'x' || (s[1] >= '0' && s[1] <= '7'))) {
                    char x = c;
                    ICONV_DECL(&x, 1);
                    assert(x == c);
                    obuf = strg_sbuf, obufv = strg_sbuf + len;
                    olen = strg_slen, olenv = strg_slen - len;
                    ICONV_DO(cd, 0, {});
                    strg_sbuf = obuf, strg_slen = olen;    /* for later reuse */
                    len += (strg_slen - len - olenv);
                } else {
#else    /* !HAVE_ICONV */
                {
#endif    /* HAVE_ICONV */
                    if (len + cbyte > strg_slen)
                        MEM_RESIZE(strg_sbuf, strg_slen += BUFUNIT);
                    if (*w) {
                        if (LITTLE != ir_cur->f.little_endian)
                            CHGENDIAN(c, sizeof(c));
                        memcpy(strg_sbuf+len,
                               &c + ((ir_cur->f.little_endian)? 0: sizeof(c)-cbyte), cbyte);
                        len += cbyte;
                    } else
                        strg_sbuf[len++] = c;
                }
                clen++;
                s = e;
                continue;
            }
            break;    /* " or NUL */
        }
        if (n == t) {
            if (len + cbyte > strg_slen)
                MEM_RESIZE(strg_sbuf, strg_slen += BUFUNIT);
            memset(strg_sbuf+len, 0, cbyte);
            len += cbyte;
            clen++;
            break;
        }
        while ((n = n->next)->id != LEX_SCON)
            if (n->id < 0)
                strg_free((arena_t *)n->spell);
        ss = s = LEX_SPELL(n);
        if (*s == 'L')
            s++;
        e = ++s;    /* skips " */
    }

    if (len % cbyte != 0)
        err_dpos(clx_pos, ERR_CONST_WIDENOTFIT);
    if (*w)
        clen = (len /= cbyte);

    if (clen - 1 > TL_STR_STD) {    /* -1 for NUL; note TL_STR_STD may warp around */
        err_dpos(clx_pos, ERR_CONST_LONGSTR);
        err_dpos(clx_pos, ERR_CONST_LONGSTRSTD, (unsigned long)TL_STR_STD);
    }

    return len;
}


#define N 0    /* no suffix */
#define U 1    /* suffix: U */
#define L 2    /* suffix: L */
#define X 3    /* suffix: UL or LU */
#define D 0    /* base: decimal */
#define H 1    /* base: octal or hexadecimal */

/*
 *  determines the type of an integer constant
 */
static const char *icon(const char *cs, ux_t n, int ovf, int base, const lmap_t *pos)
{
    static struct tab {
        unsigned long limit;
        ty_t *type;
    } tab[X+1][H+1][5];

    int suffix;
    struct tab *p;

    assert(ty_inttype);
    assert(UX_MAX >= TG_ULONG_MAX);
    assert(SMAX >= TG_LONG_MAX);

    if (tab[N][D][0].limit == 0) {
        /* no suffix, decimal */
        tab[N][D][0].limit = TG_INT_MAX;
        tab[N][D][0].type  = ty_inttype;
        tab[N][D][1].limit = TG_LONG_MAX;
        tab[N][D][1].type  = ty_longtype;
        tab[N][D][2].limit = TG_ULONG_MAX;
        tab[N][D][2].type  = ty_ulongtype;
        tab[N][D][3].limit = UX_MAX;
        tab[N][D][3].type  = ty_inttype;

        /* no suffix, octal or hex */
        tab[N][H][0].limit = TG_INT_MAX;
        tab[N][H][0].type  = ty_inttype;
        tab[N][H][1].limit = TG_UINT_MAX;
        tab[N][H][1].type  = ty_unsignedtype;
        tab[N][H][2].limit = TG_LONG_MAX;
        tab[N][H][2].type  = ty_longtype;
        tab[N][H][3].limit = TG_ULONG_MAX;
        tab[N][H][3].type  = ty_ulongtype;
        tab[N][H][4].limit = UX_MAX;
        tab[N][H][4].type  = ty_inttype;

        /* U, decimal, octal or hex */
        tab[U][H][0].limit = tab[U][D][0].limit = TG_UINT_MAX;
        tab[U][H][0].type  = tab[U][D][0].type  = ty_unsignedtype;
        tab[U][H][1].limit = tab[U][D][1].limit = TG_ULONG_MAX;
        tab[U][H][1].type  = tab[U][D][1].type  = ty_ulongtype;
        tab[U][H][2].limit = tab[U][D][2].limit = UX_MAX;
        tab[U][H][2].type  = tab[U][D][2].type  = ty_inttype;

        /* L, decimal, octal or hex */
        tab[L][H][0].limit = tab[L][D][0].limit = TG_LONG_MAX;
        tab[L][H][0].type  = tab[L][D][0].type  = ty_longtype;
        tab[L][H][1].limit = tab[L][D][1].limit = TG_ULONG_MAX;
        tab[L][H][1].type  = tab[L][D][1].type  = ty_ulongtype;
        tab[L][H][2].limit = tab[L][D][2].limit = UX_MAX;
        tab[L][H][2].type  = tab[L][D][2].type  = ty_inttype;

        /* UL or LU, decimal, octal or hex */
        tab[X][H][0].limit = tab[X][D][0].limit = TG_ULONG_MAX;
        tab[X][H][0].type  = tab[X][D][0].type  = ty_ulongtype;
        tab[X][H][1].limit = tab[X][D][1].limit = UX_MAX;
        tab[X][H][1].type  = tab[X][D][1].type  = ty_inttype;
    }

    base = (base == 10)? D: H;
    suffix = 0;

    if (((cs[0] == 'u' || cs[0] == 'U') && (cs[1] == 'l' || cs[1] == 'L')) ||
        ((cs[0] == 'l' || cs[0] == 'L') && (cs[1] == 'u' || cs[1] == 'U')))
        cs += 2, suffix = X;
    else if (*cs == 'u' || *cs == 'U')
        cs++, suffix = U;
    else if (*cs == 'l' || *cs == 'L')
        cs++, suffix = L;

    for (p = tab[suffix][base]; n > p->limit; p++)
        continue;
    if (ovf || (p->limit == UX_MAX && p->type == ty_inttype)) {
        err_dpos(pos, ERR_CONST_LARGEINT);
        n = TG_ULONG_MAX;
        tval.type = ty_ulongtype;
    } else
        tval.type = p->type;

    if (tval.type->op == TY_INT || tval.type->op == TY_LONG)
        tval.u.c.v.li = n;
    else    /* tval.type->op == TY_UNSIGNED || tval.type->op == TY_ULONG */
        tval.u.c.v.ul = n;

    return cs;
}

#undef H
#undef D
#undef X
#undef L
#undef U
#undef N


/*
 *  determines the type of a floating constant;
 *  ASSUMPTION: fp types of the host are same as those of the target
 */
static const char *fcon(const char *cs, long double ld, const lmap_t *pos)
{
    assert(ty_floattype);    /* ensures types initiailized */

    switch(*cs) {
        case 'f':
        case 'F':
            cs++;    /* skips f or F */
            if ((OVF(ld) && errno == ERANGE) || ld > TG_FLT_MAX) {
                err_dpos(pos, ERR_CONST_LARGEFP);
                ld = TG_FLT_MAX;
            } else if ((ld == 0.0 && errno == ERANGE) || (ld > 0.0 && ld < TG_FLT_MIN)) {
                err_dpos(pos, ERR_CONST_TRUNCFP);
                ld = 0.0f;
            }
            tval.type = ty_floattype;
            tval.u.c.v.f = (float)ld;
            break;
        case 'l':
        case 'L':
            cs++;    /* skips l or L */
            if ((OVF(ld) && errno == ERANGE) || ld > TG_LDBL_MAX) {
                err_dpos(pos, ERR_CONST_LARGEFP);
                ld = TG_LDBL_MAX;
            } else if ((ld == 0.0 && errno == ERANGE) || (ld > 0.0 && ld < TG_LDBL_MIN))
                err_dpos(pos, ERR_CONST_TRUNCFP);
            tval.type = ty_ldoubletype;
            tval.u.c.v.ld = (long double)ld;
            break;
        default:
            if ((OVF(ld) && errno == ERANGE) || ld > TG_DBL_MAX) {
                err_dpos(pos, ERR_CONST_LARGEFP);
                ld = (double)TG_DBL_MAX;
            } else if ((ld == 0.0 && errno == ERANGE) || (ld > 0.0 && ld < TG_DBL_MIN)) {
                err_dpos(pos, ERR_CONST_TRUNCFP);
                ld = 0.0;
            }
            tval.type = ty_doubletype;
            tval.u.c.v.d = (double)ld;
            break;
    }

    return cs;
}


/*
 *  recognizes integer or floating constants;
 *  ASSUMPTION: strtold() supported on the host
 */
static int ifcon(lex_t *t)
{
    ux_t n;
    int b, d;
    long double ld;
    int err = 0, ovf = 0;
    const char *ss, *s;
    char *p = "0123456789abcdef";    /* no const for reuse with strtold() */

    assert(t);

    ss = s = LEX_SPELL(t);

    if (*s == '.')
        goto fcon;
    n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X') &&
        isxdigit(((unsigned char *)s)[2])) {    /* 0x[0-9] */
        b = 16;
        s++;    /* skips 0 */
        while (isxdigit(*(unsigned char *)++s)) {
            d = strchr(p, tolower(*(unsigned char *)s)) - p;
            if (n & ~(UX_MAX >> 4))
                ovf = 1;
            else
                n = (n << 4) + d;
        }
        s = icon(s, n, ovf, b, t->pos);
        b = LEX_ICON;
    } else {    /* 0 or other digits */
        b = (*s == '0')? 8: 10;
        if (b == 8)
            while (isdigit(*(unsigned char *)s)) {
                d = *s++ - '0';
                if (*s == '8' || *s == '9')
                    p = (char *)s, err = 1;
                if (n & ~(UX_MAX >> 3))
                    ovf = 1;
                else
                    n = (n << 3) + d;
            }
        else    /* b == 10 */
            while (isdigit(*(unsigned char *)s)) {
                d = *s++ - '0';
                if (n > (UX_MAX - d) / 10)
                    ovf = 1;
                else
                    n = 10 * n + d;
            }

        fcon:
            if (b != 16 && (*s == '.' || *s == 'e' || *s == 'E')) {
                if (*s == '.')
                    do {
                        s++;    /* skips . and digits */
                    } while(isdigit(*s));
                if (*s == 'e' || *s == 'E') {
                    if (*++s == '-' || *s == '+')    /* skips e or E */
                        s++;    /* skips - or + */
                    if (!isdigit(*s)) {
                        err_dpos(lmap_spell(t, ss, s, s+1), ERR_CONST_NOEXPDIG);
                        err = 1;
                    }
                }
                if (!err) {
                    errno = 0;
                    ld = strtold(ss, &p);
                    s = fcon((s = p), ld, t->pos);
                }
                b = LEX_FCON;
            } else {
                if (err)
                    err_dpos(lmap_spell(t, ss, p, p+1), ERR_CONST_ILLOCTESC);
                else
                    s = icon(s, n, ovf, b, t->pos);
                b = LEX_ICON;
            }
    }

    if (*s && !err) {
        for (p = (char *)s; *p; p++)
            continue;
        err_dpos(lmap_spell(t, ss, s, p), ERR_CONST_PPNUMSFX, s, b);
        err = 1;
    }

    clx_sym = (err)? NULL: &tval;
    return b;
}


/*
 *  retrieves a token converted from a pp-token;
 *  ASSUMPTION: signed integers are compatible with unsigned ones on the host
 */
int (clx_next)(void)
{
    lex_t *t;

    while (1)
        switch((t = lst_next())->id) {
            case -1:
                strg_free((arena_t *)t->spell);
                break;
            case LEX_CREM:        /* %= */
            case LEX_CBAND:       /* &= */
            case LEX_CMUL:        /* *= */
            case LEX_CADD:        /* += */
            case LEX_CSUB:        /* -= */
            case LEX_CDIV:        /* /= */
            case LEX_CRSHFT:      /* >>= */
            case LEX_CLSHFT:      /* <<= */
            case LEX_CBXOR:       /* ^= */
            case LEX_CBOR:        /* |= */
            case '!':
            case '%':
            case '&':
            case LEX_INCR:        /* ++ */
            case '(':
            case ')':
            case '*':
            case '+':
            case ',':
            case '-':
            case '.':
            case '/':
            case LEX_DECR:        /* -- */
            case LEX_DEREF:       /* -> */
            case LEX_ANDAND:      /* && */
            case LEX_OROR:        /* || */
            case LEX_LEQ:         /* <= */
            case LEX_EQEQ:        /* == */
            case LEX_NEQ:         /* != */
            case LEX_GEQ:         /* >= */
            case LEX_RSHFT:       /* >> */
            case LEX_LSHFT:       /* << */
            case ':':
            case ';':
            case '<':
            case '=':
            case '>':
            case '?':
            case LEX_ELLIPSIS:    /* ... */
            case ']':
            case '[':
            case '^':
            case '{':
            case '|':
            case '}':
            case '~':
            case LEX_EOI:
                return t->id;
            case LEX_ID:
                return id(t);
            case LEX_CCON:
                {
                    int w = 0;
                    ux_t c = clx_ccon(t, &w);
                    tval.type = (w)? ty_wchartype: ty_inttype;
                    tval.u.c.v.ul = c;
                    clx_sym = &tval;
                }
                return t->id;
            case LEX_SCON:
                {
                    int w = 0;
                    sz_t len = scon(t, &w);
                    err_entersite(clx_pos);    /* enters with string literal */
                    tval.type = ty_array_s((!w)? ty_chartype: ty_wchartype, len);
                    err_exitsite();    /* exits from string literal */
                    tval.u.c.v.hp = strg_sbuf;
                    clx_sym = &tval;
                }
                return t->id;
            case LEX_PPNUM:
                return ifcon(t);
            case LEX_SPACE:
            case LEX_NEWLINE:
                break;
            case LEX_SHARP:      /* # */
            case LEX_DSHARP:     /* ## */
            case LEX_UNKNOWN:
                {
                    const char *p = LEX_SPELL(t);
                    err_dpos(t->pos, (*p == '\\')? ERR_LEX_STRAYBS: ERR_LEX_UNKNOWN, p);
                }
                break;
        }

    return t->id;
}


/*
 *  handles extra commas;
 *  designed to get removed with no other modifications
 */
int (clx_xtracomma)(int c, const char *s, int opt)
{
    while (clx_tc == ',') {
        err_dpos(clx_pos, ERR_LEX_EXTRACOMMA, s);
        clx_tc = clx_next();
    }
    if (clx_tc == c) {
        if (!opt)
            err_dpos(clx_pos, ERR_LEX_EXTRACOMMA, s);
        return 1;
    }

    return 0;
}

/* end of clx.h */
