/*
 *  beluga web driver;
 *    replaced by node.js version
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

#include "cgic.h"

#define PRGNAME "bweb.cgi"
#define INTMP   "out/beluga-XXXXXX"    /* name template for input file */
#define LOGSFX  ".log"                 /* suffix for log file */
#define LIMIT   (1*1024*1024)          /* max size of code to accept */

#define NELEM(a) (sizeof(a)/sizeof(*(a)))    /* number of elements in an array */

/* macros to help printing characters */
#define OUTPUTC(c) (*((pout == outbuf+sizeof(outbuf))? (outflush(), pout): pout) = (c), pout++)
#define outflush() (output(""))


static int html_flag;    /* set if HTML output has started */
static FILE *out,    /* output stream */
            *log;    /* log stream */
static char outbuf[256], *pout = outbuf;


/* system call code */
enum {
    SYS_NULL,
    SYS_SETUID,    /* setuid(uid) */
    SYS_CHROOT,    /* chroot(path) */
    SYS_CHDIR,     /* chdir(path) */
    SYS_PIPE,      /* pipe(fd) */
    SYS_LAST
};


/* generates the HTML header */
static void html_header(void)
{
    assert(out);

    fputs("<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN'\n"
          " 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n"
          "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>\n"
          "<head>\n"
          "  <title>beluga: A C Compiler</title>\n"
          "  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n"
          "  <link rel='stylesheet' type='text/css' href='beluga.css' />\n"
          "</head>\n\n"
          "<body>\n"
          "  <div id='title'>\n"
          "    <h1>\n"
          "      <img src='beluga.png' width='100px' alt='beluga logo' />\n"
          "      <i>beluga</i>: A C Compiler\n"
          "    </h1>\n"
          "  </div>\n"
          "  <div id='intro'>\n"
          "    <p>\n"
          "      Note that this site does not perform linking, not to mention execution of the\n"
          "      result. If you need to see what your program results in, use\n"
          "      <a href='http://www.codepad.org'>codepad</a> or similar services.\n"
          "    </p>\n"
          "  </div>\n\n"
          "  <div class='result'>\n"
          "    <b>GCC C Preprocessor says:</b>\n"
          "    <pre>\n",
          out);
}


/* generates the HTML tail */
static void html_tail(void)
{
    assert(out);

    fputs("    </pre>\n"
          "  </div>\n\n"
          "  <div id='footer'>\n"
          "    <p>\n"
          "      <a href='http://validator.w3.org/check?uri=referer'>"
          "<img src='http://www.w3.org/Icons/valid-xhtml10'\n"
          "       alt='Valid XHTML 1.0 Strict' height='31' width='88' /></a>\n"
          "    </p>\n"
          "  </div>\n"
          "</body>\n"
          "</html>\n",
          out);
}


/*
 *  generates the HTML separator
 */
static void html_sep(void)
{
    assert(out);

    fputs("    </pre>\n"
          "  </div>\n"
          "  <div class='result'>\n"
          "    <b><i>beluga</i> says:</b>\n"
          "    <pre>\n",
          out);
}


/*
 *  terminates when an unrecovable error occurs
 */
static void fatal(const char *fmt, ...)
{
    va_list ap;

    assert(fmt);

    if (out) {
        va_start(ap, fmt);
        fprintf(out, PRGNAME ": ");
        vfprintf(out, fmt, ap);
        va_end(ap);
    }

    if (log) {
        va_start(ap, fmt);
        fprintf(log, PRGNAME ": ");
        vfprintf(log, fmt, ap);
        va_end(ap);
    }

    if (html_flag)
        html_tail();
    fflush(NULL);

    exit(EXIT_FAILURE);
}


/*
 *  send output to out and log
 */
static void output(const char *fmt, ...)
{
    va_list ap;

    assert(fmt);

    if (pout > outbuf) {
        int n = pout - outbuf;
        pout = outbuf;
        output("%.*s", n, outbuf);
    }

    if (out) {
        va_start(ap, fmt);
        vfprintf(out, fmt, ap);
        va_end(ap);
    }

    if (log) {
        va_start(ap, fmt);
        vfprintf(log, fmt, ap);
        va_end(ap);
    }

    fflush(NULL);
}


/*
 *  executes a system call with predefined codes
 */
static void syscmd(int cmd, ...)
{
    int ret;
    va_list ap;
    const char *cmds[] = { "null", "setuid", "chroot", "chdir", "pipe" };

    assert(NELEM(cmds) == SYS_LAST);

    va_start(ap, cmd);

    ret = errno = 0;
    switch(cmd) {
        case SYS_SETUID:
            {
                uid_t uid;
                uid = va_arg(ap, uid_t);
                ret = setuid(uid);
            }
            break;
        case SYS_CHROOT:
            {
                const char *path;
                path = va_arg(ap, const char *);
                assert(path);
                ret = chroot(path);
            }
            break;
        case SYS_CHDIR:
            {
                const char *path;
                path = va_arg(ap, const char *);
                assert(path);
                ret = chdir(path);
            }
            break;
        case SYS_PIPE:
            {
                int *fd;
                fd = va_arg(ap, int *);
                ret = pipe(fd);
            }
            break;
        default:
            assert(!"not supported command -- should never reach here");
            break;
    }

    va_end(ap);

    if (ret < 0)
        fatal("%s: %s\n", cmds[cmd], strerror(errno));
}


/*
 *  makes a jail by chroot
 */
static void makejail(void)
{
    struct stat s;
    uid_t uid = getuid();

    errno = 0;
    if (stat("root", &s) < 0)
        fatal("stat: %s\n", strerror(errno));
    if (s.st_uid != uid || s.st_gid != getgid())
        fatal("permission problem on `root'\n");

    syscmd(SYS_SETUID, (uid_t)0);
    syscmd(SYS_CHROOT, "root");
    syscmd(SYS_SETUID, uid);
    syscmd(SYS_CHDIR, "root");
}


/*
 *  processes the return value of children
 */
static int retproc(int ret)
{
    int normal = 0;

    output("\n<span class=");
    if (WIFEXITED(ret)) {
        if (WEXITSTATUS(ret) == 0) {
            output("'okmsg'>completed with no errors");
            normal = 1;
        } else
            output("'fairmsg'>completed with errors");
    } else if (WIFSIGNALED(ret))
        output("'errmsg'>terminated abnormally with signal %d",
               WTERMSIG(ret));
    else if (WIFSTOPPED(ret))
        output("'errmsg'>stopped with signal %d", WSTOPSIG(ret));
    output("</span>\n");

    return normal;
}


/*
 *  prepares input to process
 */
static const char *prepin(const char *code)
{
    static char tmpn[] = INTMP;

    int n;
    int in;

    assert(code);

    n = strlen(code);

    if ((in = mkstemp(tmpn)) < 0)
        fatal("cannot create an input file\n");
    if (write(in, code, n) < n)
        fatal("cannot generate an input file\n");
    close(in);

    return tmpn;
}


/*
 *  prepares to generate log
 */
static const char *preplog(const char *tmpn)
{
    static char logn[] = INTMP LOGSFX;

    memcpy(logn, tmpn, sizeof(INTMP)-1);
    log = fopen(logn, "w");
    if (!log)
        fatal("cannot create a log file\n");
    chmod(logn, S_IRUSR | S_IWUSR);

    return logn;
}


/*
 *  gets code to compile from user input
 */
static const char *getcode(void)
{
    int n;
    char *buf;

    if (cgiFormStringSpaceNeeded("code", &n) != cgiFormSuccess || n == 0)
        fatal("no code to compile\n");
    if (LIMIT > 0 && n > LIMIT)
        fatal("code size exceeds %d bytes", LIMIT);

    buf = malloc(n);
    if (!buf)
        fatal("code too big to compile\n");

    if (cgiFormString("code", buf, n) != cgiFormSuccess)
        fatal("failed to retrieve code\n");

    return buf;
}


/*
 *  drives the preprocessor and beluga
 */
static void run(const char *code)
{
    static char buf[256],
                outn[] = "-o" INTMP LOGSFX;

    int i;
    pid_t pid;
    int fd[2];
    const char *tmpn, *logn;

    tmpn = prepin(code);
    logn = preplog(tmpn);
    sprintf(outn, "-o%s.out", tmpn);

    for (i = 0; i < 2; i++) {
        syscmd(SYS_PIPE, fd);
        errno = 0;
        if ((pid = fork()) < 0)
            fatal("fork: %s\n", strerror(errno));
        else if (pid > 0) {    /* parent */
            int n, ret;
            close(fd[1]);    /* close write channel */
            if (i)
                html_sep();
            while ((n = read(fd[0], buf, sizeof(buf))) > 0) {
                char *p;
                for (p = buf; n-- > 0; p++)
                    switch(*p) {
                        case '&':
                            output("&amp;");
                            break;
                        case '<':
                            output("&lt;");
                            break;
                        case '>':
                            output("&gt;");
                            break;
                        default:
                            OUTPUTC(*p);
                            break;
                    }
            }
            outflush();
            wait(&ret);
            close(fd[0]);
            if (!retproc(ret))
                break;
        } else {    /* child */
            close(fd[0]);    /* close read channel */
            dup2(fd[1], STDERR_FILENO);    /* send stderr to parent */
            switch(i) {
                case 0:
                    errno = 0;
                    if (execl("/usr/bin/cpp", "cpp", "-E", "-U__GNUC__", "-D__STRICT_ANSI__",
                              "-I/usr/include/", outn, tmpn, (char *)0) < 0)
                        fatal("execl: %s\n", strerror(errno));
                    break;
                case 1:
                    errno = 0;
                    if (execl("/beluga", "beluga", "-WvX", outn+2, (char *)0) < 0)
                        fatal("execl: %s\n", strerror(errno));
                    break;
                default:
                    assert(!"should never reach here");
                    break;
            }
        }
    }

    free((void *)code);
    fclose(log);
    remove(outn+2);    /* removes preprocessed file */
}


/*
 *  main function for beluga web driver
 */
int cgiMain(void)
{
    const char *code;

    out = cgiOut;

    makejail();
    cgiHeaderContentType("text/html");
    html_header();
    html_flag = 1;
    code = getcode();
    run(code);
    html_tail();

    return 0;
}

/* end of bweb.c */
