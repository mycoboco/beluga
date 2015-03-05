#
# Makefile for buliding sea-canary
#

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

RM = rm -f
RMR = rm -rf
TAR = tar
TOUCH = touch
NODE = node

ALL_CFLAGS = $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS) -lcel -lcdsl -lcbl

.c.o:
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<


BLDDIR = ../build
SRCDIR = .
BELDIR = ../src
PROFDIR = ../prof
TESTDIR = ../tst
B = $(BLDDIR)
S = $(SRCDIR)
L = $(BELDIR)
P = $(PROFDIR)
T = $(TESTDIR)

OBJS = $S/cond.o $S/ctx.o $S/expr.o $S/inc.o $S/lex.o $S/lxl.o $S/main.o $S/mcr.o $S/mg.o $S/proc.o \
       $S/strg.o $S/util.o $S/alist.o $S/err.o $S/in.o

GPROF = $P/gmon.out $P/gmon.out.* $P/gmon.sum
GCOV = $S/*.gcno $S/*.gcda *.gcov

TEST = $T/cpp/*.new $T/mcpp/*.new

M = 1
N = 0


what:
	-@echo make all clean test

all: build/sc

clean:
	$(RM) $(OBJS) $(GPROF) $(GCOV) $(TEST)

test:
	$(NODE) $T/run.js $T/cpp
	$(NODE) $T/run.js $T/mcpp

gprof:
	make clean
	CFLAGS="$(CFLAGS) -g -pg" LDFLAGS="$(LDFLAGS) -pg" make all

gcov:
	make clean
	CFLAGS="$(CFLAGS) -fprofile-arcs -ftest-coverage" \
		LDFLAGS="$(LDFLAGS) -fprofile-arcs -ftest-coverage" make all

build/sc: $(OBJS)
	$(CC) -o $B/sc $(OBJS) $(ALL_LDFLAGS)

$S/cond.o:  $S/cond.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $S/lex.h    $S/cond.h
$S/ctx.o:   $S/ctx.c    $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $S/lxl.h \
            $L/alist.h  $S/ctx.h    $S/lex.h    $S/strg.h
$S/expr.o:  $S/expr.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $S/lex.h    $S/lxl.h    $S/ctx.h    $S/mcr.h    $S/strg.h   $S/util.h \
            $S/expr.h
$S/inc.o:   $S/inc.c    $L/alist.h  $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h \
            $L/err.h    $L/xerror.h $L/in.h     $S/cond.h   $S/ctx.h    $S/lxl.h    $S/mg.h \
            $S/strg.h   $S/util.h   $S/inc.h
$S/lex.o:   $S/lex.c    $L/alist.h  $L/common.h $L/main.h   $S/lex.h    $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/strg.h   $S/util.h
$S/lxl.o:   $S/lxl.c    $S/ctx.h    $S/lxl.h    $L/alist.h  $S/lex.h    $L/xtoken.h $S/mcr.h \
            $S/strg.h
$S/main.o:  $S/main.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/inc.h    $S/cond.h   $S/lxl.h    $S/ctx.h    $S/mcr.h \
            $S/proc.h   $S/strg.h
$S/mcr.o:   $S/mcr.c    $L/alist.h  $L/common.h $L/main.h   $S/lex.h    $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/ctx.h    $S/lxl.h    $S/strg.h   $S/util.h   $S/mcr.h
$S/mg.o:    $S/mg.c     $S/mg.h
$S/proc.o:  $S/proc.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/cond.h   $S/ctx.h    $S/lxl.h    $S/expr.h   $S/inc.h \
            $S/mcr.h    $S/mg.h     $S/strg.h   $S/util.h   $S/proc.h
$S/strg.o:  $S/strg.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h
$S/util.o:  $S/util.c   $S/lex.h    $L/alist.h  $L/xtoken.h $S/util.h
$S/alist.o: $L/alist.c  $L/alist.h
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $L/alist.c
$S/err.o:   $L/err.c    $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/in.h \
            $S/inc.h    $S/cond.h   $S/lxl.h    $S/ctx.h    $L/xerror.h
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $L/err.c
$S/in.o:    $L/in.c     $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/inc.h    $S/cond.h   $S/lxl.h    $S/ctx.h
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $L/in.c
$L/err.h:    $L/xerror.h
	$(TOUCH) $@
$S/lex.h:    $L/xtoken.h
	$(TOUCH) $@
$S/tree.h:   $S/xerror.h
	$(TOUCH) $@

# end of Makefile
