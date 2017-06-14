/*
 *  configuration (cel)
 */

#include <stddef.h>    /* NULL, size_t */
#include <stdio.h>     /* FILE, EOF, fgets, getc, ungetc */
#include <ctype.h>     /* isalpha, isdigit, isspace, tolower */
#include <string.h>    /* strchr, strlen, strcpy, strcspn */
#include <stdlib.h>    /* strtod, strtol, strtoul */
#include <errno.h>     /* errno */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/memory.h"    /* MEM_ALLOC, MEM_NEW, MEM_FREE, MEM_RESIZE */
#include "cdsl/hash.h"     /* hash_string */
#include "cdsl/table.h"    /* table_t, table_new, table_get, table_put, table_free, table_map */
#include "conf.h"


#define UNUSED(id) ((void)(id))
#define BUFLEN     80              /* size of buffer to read lines */

/* checks if c is valid for section or variable names */
#define VALID_CHR(c) (isalpha(c) || isdigit(c) || (c) == '_')


/*
 *  table node for variable-value pairs
 *
 *  A table for containing variables and their values actually has a pointer to struct valnode_t as
 *  the value. To aid memory management in the library implementation, freep points to any storage
 *  that has to be freed when clean-up requested. type indicates the type of a value (string) that
 *  val points to.
 */
struct valnode_t {
    void *freep;    /* for clean-up */
    int type;       /* type of value */
    char *val;      /* value */
};


static table_t *section;    /* table for containing sections */
static table_t *current;    /* current table; see conf_section() */
static int preset;          /* set if supported sections and variables are prescribed */
static int errcode;         /* set if error has been occurred */
static int control;         /* controls some aspects of the library */


/*
 *  trims leading whitespaces
 */
static char *triml(char *s)
{
    unsigned char *p;

    assert(s);

    for (p = (unsigned char *)s; *p != '\0' && isspace(*p); p++)
        continue;

    return (char *)p;
}


/*
 *  trims trailing whitespaces;
 *  s has to point to a modifiable string
 */
static char *trimt(char *s)
{
    int lastc;
    unsigned char *p, *q;

    assert(s);

    q = NULL;
    lastc = EOF;    /* isspace(EOF) == 0 */
    for (p = (unsigned char *)s; *p != '\0'; p++) {
        if (!isspace(lastc) ^ !isspace(*p))    /* boundary found */
            q = (isspace(lastc))? NULL: p;
        lastc = *p;
    }

    if (q)
        *q = '\0';
    else
        q = p;

    return (char *)q;
}


/*
 *  finds a section or variable name from a string;
 *  p has to be a modifiable string
 */
static int sepunit(char *p, char **pp)
{
    unsigned char *q;

    /* assert(p) done by triml() */
    assert(pp);

    errcode = CONF_ERR_OK;

    q = (unsigned char *)(p = triml(p));
    trimt(p);

    while (VALID_CHR(*q))
            q++;

    if (*q != '\0') {
        if (isspace(*q))
            errcode = CONF_ERR_SPACE;
        else
            errcode = CONF_ERR_CHAR;
    }
    *q = '\0';

    *pp = p;

    return errcode;
}


/*
 *  finds section and variable names from a string;
 *  sep() assumes that the given string is modifiable
 */
static int sep(char *var, char **psec, char **pvar)
{
    char *p;

    assert(var);
    assert(psec);
    /* assert(pvar) done by sepunit() */

    errcode = CONF_ERR_OK;

    p = strchr(var, '.');
    if (p) {    /* section given */
        *p = '\0';
        errcode = sepunit(var, psec);
        p++;
    } else {    /* current section */
        *psec = NULL;
        p = var;
    }

    if (errcode == CONF_ERR_OK) {
        errcode = sepunit(p, pvar);
        if (errcode == CONF_ERR_OK && *(unsigned char *)*pvar == '\0')    /* empty variable */
            errcode = CONF_ERR_VAR;
    }

    return errcode;
}


/*
 *  removes a comment;
 *  s has to be a modifiable string
 */
static void cmtrem(char *s)
{
    unsigned char *p;

    assert(s);

    for (p = (unsigned char *)s; *p != '\0'; p++)
        if (*p == ';' || *p == '#') {
            *p++ = ' ';
            *p = '\0';
        }
}


/*
 *  converts any uppercase letters in a string to corresponding lowercase ones;
 *  s has to be a modifiable string
 */
static const char *lower(char *s)
{
    unsigned char *p;

    assert(s);

    for (p = (unsigned char *)s; *p != '\0'; p++)
        *p = tolower(*p);

    return s;
}


/*
 *  constructs a default set for configuration variables
 *
 *  conf_preset() cannot be implemented with conf_set() since they differ in handling an omitted
 *  section name. conf_section() cannot affect conf_preset() because the former is not callable
 *  before the latter. Thus, conf_preset() can safely assume that a section name is given properly
 *  whenever necessary. On the other hand, because conf_set() is affected by conf_section(), a
 *  variable name without a section name and a period should be recognized as referring to the
 *  current section, not to the global section.
 *
 *  TODO:
 *    - some adjustment on arguments to table_new() is necessary;
 *    - considering changes to the format of a configuration file as a program to accept it is
 *      upgraded, making it a recoverable error to encounter a non-preset section or variable name
 *      would be useful; this enables an old version of the program to accept a new configuration
 *      file with diagnostics.
 */
int (conf_preset)(const conf_t *tab, int ctrl)
{
    size_t len;
    char *pbuf, *sec, *var;
    const char *hkey;
    table_t *t;
    struct valnode_t *pnode;

    assert(!section);
    assert(tab);

    control = ctrl;
    section = table_new(0, NULL, NULL);
    errcode = CONF_ERR_OK;

    for (; tab->var; tab++) {
        assert(tab->defval);
        assert(tab->type == CONF_TYPE_BOOL || tab->type == CONF_TYPE_INT ||
               tab->type == CONF_TYPE_UINT || tab->type == CONF_TYPE_REAL ||
               tab->type == CONF_TYPE_STR);

        len = strlen(tab->var);
        pbuf = strcpy(MEM_ALLOC(len+1 + strlen(tab->defval)+1), tab->var);
        if (sep(pbuf, &sec, &var) != CONF_ERR_OK) {
            MEM_FREE(pbuf);
            break;    /* sep sets errcode */
        }
        if (!sec)    /* means global section in this case */
            sec = "";    /* lower modifies sec only when necessary */
        hkey = hash_string((control & CONF_OPT_CASE)? sec: lower(sec));
        t = table_get(section, hkey);
        if (!t) {    /* new section */
            t = table_new(0, NULL, NULL);
            table_put(section, hkey, t);
        }
        MEM_NEW(pnode);
        pnode->type = tab->type;
        pnode->freep = pbuf;
        pnode->val = strcpy(pbuf + len+1, tab->defval);
        table_put(t, hash_string((control & CONF_OPT_CASE)? var: lower(var)), pnode);
    }

    preset = 1;

    return errcode;
}


/*
 *  returns a character for a escape sequence character
 */
static int escseq(int c)
{
    switch(c) {
        case '\'':    /* \' */
            return '\'';
        case '\"':    /* \" */
            return '\"';
        case '\\':    /* \\ */
            return '\\';
        case '0':    /* null char */
            return '\0';
        case 'a':    /* \a */
            return '\a';
        case 'b':    /* \b */
            return '\b';
        case 'f':    /* \f */
            return '\f';
        case 'n':    /* \n */
            return '\n';
        case 'r':    /* \r */
            return '\r';
        case 't':    /* \t */
            return '\t';
        case 'v':    /* \v */
            return '\v';
        case ';':    /* \; */
            return ';';
        case '#':    /* \# */
            return '#';
        case '=':    /* \= */
            return '=';
        default:    /* unrecognized */
            break;
    }

    return c;
}


/*
 *  reads a configuration file and constructs the configuration data
 *
 *  A tricky construct using do-while for reading lines from a file is to allow for a configuration
 *  file that does not end with a newline. Asserting that BUFLEN is greater than 1 is necessary to
 *  prevent fgets() from returning an empty string even if there are still characters unread.

 *  TODO:
 *    - some adjustment on arguments to table_new() is necessary
 */
size_t (conf_init)(FILE *fp, int ctrl)
{
    char *p;
    size_t buflen, len;
    size_t lineno, nlineno;
    const char *hkey;
    table_t *tab;
    struct valnode_t *pnode;

    assert(fp);

    assert(preset || !section);
    assert(!current);
    assert(BUFLEN > 1);

    if (!preset) {
        control = ctrl;    /* if preset, ctrl ignored */
        section = table_new(0, NULL, NULL);
    }

    tab = NULL;
    errcode = CONF_ERR_OK;
    lineno = nlineno = 0;
    len = 0;
    p = MEM_ALLOC(buflen=BUFLEN);
    *p = '\0';

    do {
        assert(buflen - len > 1);
        fgets(p+len, buflen-len, fp);
        if (ferror(fp)) {
            errcode = CONF_ERR_IO;
            break;
        }
        len += strlen(p+len);
        if (len == 0)    /* EOF and no remaining data */
            break;
        if (len > 2 && p[len-2] == '\\' && p[len-1] == '\n') {    /* line splicing */
            int c;
            nlineno++;
            len -= 2;
            if (len > 0 && isspace(p[len-1])) {
                while (--len > 0 && isspace(p[len-1]))
                    continue;
                p[len++] = ' ';
            }
            if ((c = getc(fp)) != EOF && c != '\n' && isspace(c)) {
                if (p[len-1] != ' ')
                    p[len++] = ' ';
                while ((c = getc(fp)) != EOF && c != '\n' && isspace(c))
                        continue;
            } else if (c == EOF) {    /* no following line for slicing */
                lineno += nlineno;
                errcode = CONF_ERR_BSLASH;
                goto retcode;
            }
            ungetc(c, fp);
            p[len] = '\0';
            /* at worst, backslash is replaced with space and newline with null, thus room for two
               characters, which guarantees buflen-len greater than 1 */
        } else if (p[len-1] == '\n' || feof(fp)) {    /* line completed */
            int type;
            char *s, *t, *var, *val;

            lineno++;
            if (p[len-1] == '\n')
                p[--len] = '\0';    /* in fact, no need to adjust len */

            var = triml(p);
            switch(*var) {
                case '[':    /* section specification */
                    cmtrem(++var);
                    s = strchr(var, ']');
                    if (!s) {
                        errcode = CONF_ERR_LINE;
                        goto retcode;
                    }
                    *s = '\0';
                    if (sepunit(var, &var) != CONF_ERR_OK)
                        goto retcode;    /* sepunit sets errcode */
                    hkey = hash_string((control & CONF_OPT_CASE)? var: lower(var));
                    tab = table_get(section, hkey);
                    if (!tab) {
                        if (preset) {
                            errcode = CONF_ERR_SEC;
                            goto retcode;
                        }
                        tab = table_new(0, NULL, NULL);
                    }
                    table_put(section, hkey, tab);
                    /* no break */
                case '\0':    /* empty line */
                case '#':     /* comment-only line */
                case ';':
                    break;    /* reuses existing buffer */
                default:    /* variable = value */
                    val = p + strcspn(p, "#;");
                    s = strchr(var, '=');
                    if (!s || val < s) {
                        errcode = CONF_ERR_LINE;
                        goto retcode;
                    }
                    *s = '\0';
                    if (sepunit(var, &var) != CONF_ERR_OK)
                        goto retcode;    /* sepunit() sets errcode */
                    if (*(unsigned char *)var == '\0') {    /* empty variable name */
                        errcode = CONF_ERR_VAR;
                        goto retcode;
                    }
                    val = triml(++s);
                    if (*val == '"' || *val == '\'') {    /* quoted */
                        char end = *val;    /* remembers for match */

                        t = s = ++val;    /* starts copying from s to t */
                        do {
                            switch(*s) {
                                case '\\':    /* escape sequence */
                                    if (control & CONF_OPT_ESC)
                                        *(unsigned char *)t++ = escseq(*(unsigned char *)++s);
                                    else {
                                        *t++ = '\\';
                                        *(unsigned char *)t++ = *(unsigned char *)++s;
                                    }
                                    break;
                                case '\0':    /* unclosed ' or " */
                                    errcode = CONF_ERR_LINE;
                                    goto retcode;
                                case '\'':
                                case '\"':
                                    if (*s == end) {
                                        *t = '\0';
                                        break;
                                    }
                                    /* no break */
                                default:    /* literal copy */
                                    *(unsigned char *)t++ = *(unsigned char *)s;
                                    break;
                            }
                            s++;
                        } while(*(unsigned char *)t != '\0');
                        /* checks if any trailing invalid char */
                        cmtrem(s);
                        trimt(s);
                        if (*(unsigned char *)s != '\0') {
                            errcode = CONF_ERR_LINE;
                            goto retcode;
                        }
                    } else {    /* unquoted */
                        cmtrem(val);
                        trimt(val);
                    }

                    if (!tab) {    /* global section */
                        hkey = hash_string("");
                        tab = table_get(section, hkey);
                        if (!tab) {
                            if (preset) {
                                errcode = CONF_ERR_SEC;
                                goto retcode;
                            }
                            tab = table_new(0, NULL, NULL);
                        }
                        table_put(section, hkey, tab);
                    }
                    hkey = hash_string((control & CONF_OPT_CASE)? var: lower(var));
                    if (preset) {
                        pnode = table_get(tab, hkey);
                        if (!pnode) {
                            errcode = CONF_ERR_VAR;
                            goto retcode;
                        }
                        type = pnode->type;
                    } else
                        type = CONF_TYPE_STR;
                    MEM_NEW(pnode);
                    pnode->freep = p;
                    pnode->type = type;
                    pnode->val = val;
                    pnode = table_put(tab, hkey, pnode);
                    if (pnode) {    /* value overwritten, thus free */
                        MEM_FREE(pnode->freep);
                        MEM_FREE(pnode);
                    }

                    p = MEM_ALLOC(buflen=BUFLEN);    /* uses new buffer */
                    break;
            }
            len = 0;
            *p = '\0';
            lineno += nlineno;    /* adjusts line number */
            nlineno = 0;
        } else    /* expands buffer */
            MEM_RESIZE(p, buflen+=BUFLEN);
    } while(!feof(fp));

    retcode:
        MEM_FREE(p);
        return (errcode != CONF_ERR_OK)? lineno: 0;
}


/*
 *  retrieves a pointer to a value with a section/variable name
 */
static void *valget(const char *secvar)
{
    size_t len;
    char buf[30], *pbuf;
    char *sec, *var;
    table_t *tab;
    const char *hkey;
    void *pnode;

    assert(secvar);

    /* sep() always clears errcode */

    len = strlen(secvar);
    pbuf = strcpy((len > sizeof(buf)-1)? MEM_ALLOC(len+1): buf, secvar);

    if (sep(pbuf, &sec, &var) != CONF_ERR_OK) {
        if (pbuf != buf)
            MEM_FREE(pbuf);
        return NULL;    /* sep sets errcode */
    }

    if (!sec) {    /* current selected */
        if (!current)    /* no current section, thus global */
            tab = table_get(section, hash_string(""));
        else
            tab = current;
    } else    /* section name given */
        tab = table_get(section, hash_string((control & CONF_OPT_CASE)? sec: lower(sec)));

    if (!tab) {
        if (pbuf != buf)
            MEM_FREE(pbuf);
        errcode = CONF_ERR_SEC;
        return NULL;
    }

    hkey = hash_string((control & CONF_OPT_CASE)? var: lower(var));
    if (pbuf != buf)
        MEM_FREE(pbuf);

    pnode = table_get(tab, hkey);
    if (!pnode)
        errcode = CONF_ERR_VAR;

    return pnode;
}


/*
 *  converts a string based on a type
 */
const void *conf_conv(const char *val, int type)
{
    /* provides storage for conversion result */
    static int boolean;              /* for CONF_TYPE_BOOL */
    static long inumber;             /* for CONF_TYPE_INT */
    static unsigned long unumber;    /* for CONF_TYPE_UINT */
    static double fnumber;           /* for CONF_TYPE_REAL */

    char *endptr;

    assert(val);

    errcode = CONF_ERR_OK;
    errno = 0;
    switch(type) {
        case CONF_TYPE_BOOL:
            while (*(unsigned char *)(val) != '\0' && isspace(*(unsigned char *)val))
                val++;    /* ignores leading spaces */
            if (*val == 't' || *val == 'T' || *val == 'y' || *val == 'Y' || *val == '1')
                boolean = 1;
            else
                boolean = 0;
            return &boolean;
        case CONF_TYPE_INT:
            inumber = strtol(val, &endptr, 0);
            if (*(unsigned char *)endptr != '\0' || errno)
                break;
            else
                return &inumber;
        case CONF_TYPE_UINT:
            unumber = strtoul(val, &endptr, 0);
            if (*(unsigned char *)endptr != '\0' || errno)
                break;
            else
                return &unumber;
        case CONF_TYPE_REAL:
            fnumber = strtod(val, &endptr);
            if (*(unsigned char *)endptr != '\0' || errno)
                break;
            else
                return &fnumber;
        case CONF_TYPE_STR:
            return val;
        default:
            assert(!"unknown conversion requested -- should never reach here");
            break;
    }

    errcode = CONF_ERR_TYPE;
    return NULL;
}


/*
 *  retrieves a value from a section/variable name
 */
const void *(conf_get)(const char *var)
{
    struct valnode_t *pnode;

    /* assert(var) done by valget() */

    if ((pnode = valget(var)) == NULL)
        return NULL;    /* valget() sets errcode */

    return conf_conv(pnode->val, pnode->type);    /* conf_conv() sets errcode */
}


/*
 *  retrieves a boolean value from a section/variable name
 */
int (conf_getbool)(const char *var, int errval)
{
    struct valnode_t *pnode;

    /* assert(var) done by valget() */

    pnode = valget(var);

    if (!pnode || pnode->type != CONF_TYPE_BOOL)
        return errval;    /* valget() sets errcode */

    return *(const int *)conf_conv(pnode->val, CONF_TYPE_BOOL);    /* conf_conv() sets errcode */
}


/*
 *  retrieves an integral value from a section/variable name
 */
long (conf_getint)(const char *var, long errval)
{
    const void *p;
    struct valnode_t *pnode;

    /* assert(val) done by valget() */

    pnode = valget(var);

    if (!pnode || pnode->type != CONF_TYPE_INT ||
        (p = conf_conv(pnode->val, CONF_TYPE_INT)) == NULL)
        return errval;    /* valget() and confconv() set errcode */

    return *(const long *)p;
}


/*
 *  retrieves an unsigned integral value from a section/variable name
 */
unsigned long (conf_getuint)(const char *var, unsigned long errval)
{
    const void *p;
    struct valnode_t *pnode;

    /* assert(var) done by valget() */

    pnode = valget(var);

    if (!pnode || pnode->type != CONF_TYPE_UINT ||
        (p = conf_conv(pnode->val, CONF_TYPE_UINT)) == NULL)
        return errval;    /* valget() and conf_conv() set errcode */

    return *(const unsigned long *)p;
}


/*
 *  retrieves a real value from a section/variable name.
 */
double (conf_getreal)(const char *var, double errval)
{
    const void *p;
    struct valnode_t *pnode;

    /* assert(val) done by valget() */

    pnode = valget(var);

    if (!pnode || pnode->type != CONF_TYPE_REAL ||
        (p = conf_conv(pnode->val, CONF_TYPE_REAL)) == NULL)
        return errval;    /* valget() and conf_conv() set errcode */

    return *(const double *)p;
}


/*
 *  retrieves a string from a section/variable name
 */
const char *(conf_getstr)(const char *var)
{
    struct valnode_t *pnode;

    /* assert(var) done by valget() */

    pnode = valget(var);

    if (!pnode || pnode->type != CONF_TYPE_STR)
        return NULL;    /* valget() sets errcode */

    return conf_conv(pnode->val, CONF_TYPE_STR);    /* conf_conv() sets errcode */
}


/*
 *  inserts or replaces a value associated with a variable
 *
 *  TODO:
 *    - some adjustment on arguments to table_new() is necessary
 */
int (conf_set)(const char *secvar, const char *value)
{
    size_t len;
    char buf[30], *pbuf;
    char *sec, *var;
    table_t *tab;
    const char *hkey;
    struct valnode_t *pnode;

    assert(secvar);
    assert(value);

    /* sep() always clears errcode */

    len = strlen(secvar);
    pbuf = strcpy((len > sizeof(buf)-1)? MEM_ALLOC(len+1): buf, secvar);

    if (sep(pbuf, &sec, &var) != CONF_ERR_OK) {
        if (pbuf != buf)
            MEM_FREE(pbuf);
        return errcode;    /* sep sets errcode */
    }

    if (!sec) {    /* current selected */
        if (!current) {    /* no current section, thus global */
            hkey = hash_string("");    /* in case table for global not yet allocated */
            tab = table_get(section, hkey);
        } else
            tab = current;
    } else {    /* section name given */
        hkey = hash_string((control & CONF_OPT_CASE)? sec: lower(sec));
        tab = table_get(section, hkey);
    }

    if (!tab) {    /* new section */
        if (preset) {
            if (pbuf != buf)
                MEM_FREE(pbuf);
            return (errcode = CONF_ERR_SEC);
        } else {    /* adds it */
            tab = table_new(0, NULL, NULL);
            table_put(section, hkey, tab);
        }
    }

    assert(tab);

    hkey = hash_string((control & CONF_OPT_CASE)? var: lower(var));
    if (pbuf != buf)
        MEM_FREE(pbuf);

    pnode = table_get(tab, hkey);
    if (!pnode) {    /* new variable */
        if (preset)
            return (errcode = CONF_ERR_VAR);
        else {    /* inserts */
            MEM_NEW(pnode);
            pnode->type = CONF_TYPE_STR;
            table_put(tab, hkey, pnode);
        }
    } else    /* replaces, thus reuses pnode */
        MEM_FREE(pnode->freep);
    pnode->val = pnode->freep = strcpy(MEM_ALLOC(strlen(value)+1), value);

    return errcode;
}


/*
 *  sets the current section
 */
int (conf_section)(const char *sec)
{
    size_t len;
    char buf[30], *pbuf;
    const char *hkey;
    table_t *tab;

    assert(sec);

    /* sepunit() always clears errcode */

    len = strlen(sec);
    pbuf = strcpy((len > sizeof(buf)-1)? MEM_ALLOC(len+1): buf, sec);

    if (sepunit(pbuf, &pbuf) != CONF_ERR_OK) {
        if (pbuf != buf)
            MEM_FREE(pbuf);
        return errcode;    /* sepunit sets errcode */
    }

    hkey = hash_string((control & CONF_OPT_CASE)? pbuf: lower(pbuf));
    if (pbuf != buf)
        MEM_FREE(pbuf);

    tab = table_get(section, hkey);
    if (tab)
        current = tab;
    else
        errcode = CONF_ERR_SEC;

    return errcode;
}


/*
 *  provides a callback for conf_free()
 */
static void tabfree(const void *key, void **value, void *cl)
{
    UNUSED(key);

    if (cl == section) {    /* for section table */
        table_t *t = *value;
        table_map(t, tabfree, *value);
        table_free(&t);
    } else {    /* for var=val tables */
        struct valnode_t *pnode = *value;
        assert(pnode);
        MEM_FREE(pnode->freep);
        MEM_FREE(pnode);
    }
}


/*
 *  deallocates the stroage for the configuration data
 */
void (conf_free)(void)
{
    table_map(section, tabfree, section);
    table_free(&section);
    current = NULL;
    preset = 0;
    errcode = CONF_ERR_OK;
    control = 0;
}


/*
 *  resets the hash table using hash_reset()
 */
void (conf_hashreset)(void)
{
    hash_reset();
}


/*
 *  returns an error code
 */
int (conf_errcode)(void)
{
    return errcode;
}


/*
 *  returns an error message
 */
const char *(conf_errstr)(int code)
{
    static const char *s[] = {
        "everything is okay",                /* CONF_ERR_OK */
        "file not found",                    /* CONF_ERR_NOFILE */
        "I/O error occurred",                /* CONF_ERR_IO */
        "space in section/variable name",    /* CONF_ERR_SPACE */
        "invalid character encountered",     /* CONF_ERR_CHAR */
        "invalid line encountered",          /* CONF_ERR_LINE */
        "no following line for splicing",    /* CONF_ERR_BSLASH */
        "section not found",                 /* CONF_ERR_SEC */
        "variable not found",                /* CONF_ERR_VAR */
        "data type mismatch",                /* CONF_ERR_TYPE */
    };

    assert(CONF_ERR_MAX == sizeof(s)/sizeof(*s));
    assert(code >= 0 || code < sizeof(s)/sizeof(*s));

    return s[code];
}


#if 0    /* test code */
#include <assert.h>    /* assert */
#include <stddef.h>    /* size_t, NULL */
#include <stdio.h>     /* FILE, fopen, fclose, printf, fprintf, stderr, puts, rewind, EOF */
#include <stdlib.h>    /* EXIT_FAILURE */

#include <cel/conf.h>    /* conf_t, CONF_TYPE_*, CONF_ERR_*, conf_preset, conf_init, conf_get,
                            conf_errset, conf_errcode, conf_section, conf_set, conf_free */

static const char *cfname = "test.conf";

static void print(const char *var, int type, int preset)
{
    const void *p;
    const char *boolean[] = { "true", "false" };

    assert(var);

    p = conf_get(var);
    if (!p)
        fprintf(stderr, "test: %s - %s\n", var, conf_errstr(conf_errcode()));
    else {
        if (!preset)
            switch(type) {
                case CONF_TYPE_BOOL:
                    printf("%s = %s\n", var, boolean[*(const int *)p]);
                    break;
                case CONF_TYPE_INT:
                    printf("%s = %ld\n", var, *(const long *)p);
                    break;
                case CONF_TYPE_UINT:
                    printf("%s = %lu\n", var, *(const unsigned long *)p);
                    break;
                case CONF_TYPE_REAL:
                    printf("%s = %e\n", var, *(const double *)p);
                    break;
                case CONF_TYPE_STR:
                    printf("%s = %s\n", var, (const char *)p);
                    break;
                default:
                    assert(!"invalid type -- should never reach here");
                    break;
            }
        else    /* ignores type and always assumes CONF_TYPE_STR */
            printf("%s = %s\n", var, (const char *)p);
    }
}

int main(void)
{
    conf_t tab[] = {
        /* global section */
        "VarBool",          CONF_TYPE_BOOL, "yes",
        "VarInt",           CONF_TYPE_INT,  "255",
        "VarUint",          CONF_TYPE_UINT, "0xFFFF",
        "VarReal",          CONF_TYPE_REAL, "3.14159",
        "VarStr",           CONF_TYPE_STR,  "Global VarStr Default",
        /* Section1 */
        "Section1.VarBool", CONF_TYPE_BOOL, "FALSE",
        "Section1.VarInt",  CONF_TYPE_INT,  "254",
        "Section1.VarUint", CONF_TYPE_UINT, "0xFFFE",
        "Section1.VarReal", CONF_TYPE_REAL, "3.14160",
        "Section1.VarStr",  CONF_TYPE_STR,  "Section1.VarStr Default",
        /* Section2 */
        "Section2.VarBool", CONF_TYPE_BOOL, "TRUE",
        "Section2.VarInt",  CONF_TYPE_INT,  "253",
        "Section2.VarUint", CONF_TYPE_UINT, "0xFFFD",
        "Section2.VarReal", CONF_TYPE_REAL, "3.14161",
        "Section2.VarStr",  CONF_TYPE_STR,  "Section2.VarStr Default",
        /* Section3 */
        "Section3.VarBool", CONF_TYPE_BOOL, "1",
        "Section3.VarInt",  CONF_TYPE_INT,  "252",
        "Section3.VarUint", CONF_TYPE_UINT, "0xFFFC",
        "Section3.VarReal", CONF_TYPE_REAL, "3.14162",
        "Section3.VarStr",  CONF_TYPE_STR,  "Section3.VarStr Default",
        NULL,
    };
    int i;
    size_t line;
    FILE *fp;
    const char *s = "# configuration file example\n"
                    "\n"
                    "\t[\tSection1]    # Section 1\n"
                    "VarReal      \\\n"
                    "    = 0.314\\\n"
                    "159\n"
                    "\n"
                    "\n"
                    "[Section2\t]\t; Section 2\n"
                    "\n"
                    "        Var\\\n"
                    "Str = \"Section#2.VarStr\\tNew\\\n"
                    "\t    default\"     ; set VarStr of Section 2\n"
                    "\n"
                    "[       ]    # global section\n"
                    "VarUint     =      \"010\"\n"
                    "VarStr=\"\"\n"
                    "VarStr=This does not use \"\t# unquoted value\n";

    if (conf_preset(tab, CONF_OPT_CASE | CONF_OPT_ESC) != CONF_ERR_OK) {
        fprintf(stderr, "test: %s\n", conf_errstr(conf_errcode()));
        conf_free();
        conf_hashreset();
        return EXIT_FAILURE;
    }

    for (i = 0; i < 2; i++) {
        fp = fopen(cfname, "r");
        if (!fp) {    /* creates a configuration file */
            fp = fopen(cfname, "w+");
            assert(fp);
            assert(fputs(s, fp) != EOF);
            rewind(fp);
        }

        /* starts testing */
        line = conf_init(fp, (i == 0)? CONF_OPT_CASE | CONF_OPT_ESC: 0);
        fclose(fp);    /* fp is not necessary any more */

        if (line != 0) {
            fprintf(stderr, "test:%s:%ld: %s\n", cfname, (unsigned long)line,
                    conf_errstr(conf_errcode()));
            conf_free();
            conf_hashreset();
            return EXIT_FAILURE;
        }

        printf("%s* Testing in %spreset mode with%s CONF_OPT_CASE and CONF_OPT_ESC...\n",
               (i == 0)? "": "\n", (i == 0)? "": "non-", (i == 0)? "": "out");

        puts("\n[global section]");
        print("VarBool", CONF_TYPE_BOOL, i);
        print("VarInt", CONF_TYPE_INT, i);
        print("VarUint", CONF_TYPE_UINT, i);
        print("VarReal", CONF_TYPE_REAL, i);
        print("VarStr", CONF_TYPE_STR, i);

        puts("");
        print("Section1.VarBool", CONF_TYPE_BOOL, i);
        print("Section1.VarInt", CONF_TYPE_INT, i);
        print("Section1.VarUint", CONF_TYPE_UINT, i);
        print("Section1.VarReal", CONF_TYPE_REAL, i);
        print("Section1.VarStr", CONF_TYPE_STR, i);

        printf("\n[switching to Section2 - %s]\n",
               (conf_section("Section2") == CONF_ERR_OK)?  "done": "failed");
        print("VarBool", CONF_TYPE_BOOL, i);
        print("VarInt", CONF_TYPE_INT, i);
        print("VarUint", CONF_TYPE_UINT, i);
        print("VarReal", CONF_TYPE_REAL, i);
        print("VarStr", CONF_TYPE_STR, i);

        puts("");
        print("Section3.VarBool", CONF_TYPE_BOOL, i);
        print("Section3.VarInt", CONF_TYPE_INT, i);
        print("Section3.VarUint", CONF_TYPE_UINT, i);
        print("Section3.VarReal", CONF_TYPE_REAL, i);
        print("Section3.VarStr", CONF_TYPE_STR, i);

        printf("\n[setting VarStr - %s]\n",
               (conf_set("VarStr", "Set in run-time") == CONF_ERR_OK)? "done": "failed");
        printf("[setting Section4.VarReal - %s]\n",
               (conf_set("Section4.VarReal", "2.718281") == CONF_ERR_OK)? "done": "failed");

        print("VarStr", CONF_TYPE_STR, i);
        print("Section4.VarReal", CONF_TYPE_REAL, i);

        conf_free();
        conf_hashreset();    /* conf_free() does not reset hash table */
    }

    return 0;
}
#endif    /* disabled */

/* end of conf.c */
