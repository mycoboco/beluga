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

BLDDIR = ../build
SRCDIR = .
BELDIR = ../src
PROFDIR = ../prof
TESTDIR = ../tst
DEPSDIR = ../deps
B = $(BLDDIR)
S = $(SRCDIR)
L = $(BELDIR)
P = $(PROFDIR)
T = $(TESTDIR)
D = $(DEPSDIR)

OV = 1.0.0
M = 1
N = 0


ALL_CFLAGS = $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)

.c.o:
	$(CC) -I$D/ocelot-$(OV)/build/include -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<


OBJS = $S/cond.o $S/ctx.o $S/expr.o $S/inc.o $S/lex.o $S/lxl.o $S/main.o $S/mcr.o $S/mg.o \
       $S/prgm.o $S/proc.o $S/strg.o $S/util.o $S/alist.o $S/err.o $S/in.o

DEPS = $D/ocelot-$(OV)/build/lib/libcel.a $D/ocelot-$(OV)/build/lib/libcdsl.a \
       $D/ocelot-$(OV)/build/lib/libcbl.a

GPROF = $P/gmon.out $P/gmon.out.* $P/gmon.sum
GCOV = $S/*.gcno $S/*.gcda *.gcov

TEST = $T/cpp/*.new $T/mcpp/*.new


what:
	-@echo make all clean test

all: deps build/sc

clean:
	$(RM) $(OBJS) $(GPROF) $(GCOV) $(TEST)
	$(MAKE) -C $D/ocelot-$(OV) clean

test:
	$(NODE) $T/run.js $T/cpp
	$(NODE) $T/run.js $T/mcpp

gprof:
	$(MAKE) clean
	CFLAGS="$(CFLAGS) -g -pg" LDFLAGS="$(LDFLAGS) -pg" $(MAKE) all

gcov:
	$(MAKE) clean
	CFLAGS="$(CFLAGS) -fprofile-arcs -ftest-coverage" \
		LDFLAGS="$(LDFLAGS) -fprofile-arcs -ftest-coverage" $(MAKE) all

build/sc: $(OBJS)
	$(CC) $(ALL_LDFLAGS) -o $B/sc $(OBJS) $(DEPS)

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
            $S/proc.h   $S/strg.h   $S/util.h   $S/../version.h
$S/mcr.o:   $S/mcr.c    $L/alist.h  $L/common.h $L/main.h   $S/lex.h    $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/ctx.h    $S/lxl.h    $S/strg.h   $S/util.h   $S/mcr.h
$S/mg.o:    $S/mg.c     $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $S/inc.h \
            $S/cond.h   $S/lxl.h    $S/ctx.h    $S/mcr.h    $S/strg.h   $S/mg.h
$S/prgm.o:  $S/prgm.c   $S/lex.h    $L/alist.h  $L/xtoken.h $S/lxl.h    $S/ctx.h    $S/mg.h \
            $S/util.h   $S/prgm.h
$S/proc.o:  $S/proc.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/cond.h   $S/ctx.h    $S/lxl.h    $S/expr.h   $S/inc.h \
            $S/mcr.h    $S/mg.h     $S/prgm.h   $S/strg.h   $S/util.h   $S/proc.h
$S/strg.o:  $S/strg.c   $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/strg.h
$S/util.o:  $S/util.c   $S/lex.h    $L/alist.h  $L/xtoken.h $S/util.h
$S/alist.o: $L/alist.c  $L/alist.h
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $L/alist.c
$S/err.o:   $L/err.c    $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/in.h \
            $S/cond.h   $S/inc.h    $S/lxl.h    $S/ctx.h    $L/err.h    $L/xerror.h
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $L/err.c
$S/in.o:    $L/in.c     $L/common.h $L/main.h   $S/lex.h    $L/alist.h  $L/xtoken.h $L/err.h \
            $L/xerror.h $L/in.h     $S/inc.h    $S/cond.h   $S/lxl.h    $S/ctx.h
	$(CC) -DSEA_CANARY -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $L/in.c

deps:
	$(MAKE) -C $D/ocelot-$(OV) static

# end of Makefile
