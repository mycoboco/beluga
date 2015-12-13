#
# Makefile for buliding beluga
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
DRVDIR = ../bcc
PROFDIR = ../prof
TESTDIR = ../tst
DEPSDIR = ../deps
B = $(BLDDIR)
S = $(SRCDIR)
R = $(DRVDIR)
P = $(PROFDIR)
T = $(TESTDIR)
D = $(DEPSDIR)

OV = nightly


ALL_CFLAGS = -I$D/ocelot-$(OV)/build/include $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)

.c.o:
	$(CC) -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<


OBJS = $S/alist.o $S/cgr.o $S/dag.o $S/decl.o $S/enode.o $S/err.o $S/expr.o $S/gen.o $S/in.o \
       $S/init.o $S/ir.o $S/lex.o $S/main.o $S/op.o $S/reg.o $S/simp.o $S/stmt.o $S/strg.o \
       $S/sym.o $S/tree.o $S/ty.o $S/bnull.o $S/bx86t.o $S/bx86l.o

DEPS = $D/ocelot-$(OV)/build/lib/libcel.a $D/ocelot-$(OV)/build/lib/libcdsl.a \
       $D/ocelot-$(OV)/build/lib/libcbl.a

GPROF = $P/gmon.out $P/gmon.out.* $P/gmon.sum
GCOV = $S/*.gcno $S/*.gcda *.gcov

TEST = $T/diag/*.new $T/asm/*.s


what:
	-@echo make all clean test

all: deps $B/beluga

clean:
	$(RM) $(OBJS) $(GPROF) $(GCOV) $(TEST)
	$(MAKE) -C $D/ocelot-$(OV) clean

test:
	$(NODE) $T/run.js $T/diag && $(NODE) $T/run.js $T/asm

gprof:
	$(MAKE) clean
	CFLAGS="$(CFLAGS) -g -pg" LDFLAGS="$(LDFLAGS) -pg" $(MAKE) all

gcov:
	$(MAKE) clean
	CFLAGS="$(CFLAGS) -fprofile-arcs -ftest-coverage" \
		LDFLAGS="$(LDFLAGS) -fprofile-arcs -ftest-coverage" $(MAKE) all

$B/beluga: $(OBJS)
	$(CC) $(ALL_LDFLAGS) -o $B/beluga $(OBJS) $(DEPS)

$S/alist.o: $S/alist.c  $S/alist.h
$S/bnull.o: $S/bnull.c  $S/bnull.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/gen.h \
            $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h
$S/bx86t.o: $S/bx86t.c  $S/bnull.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/gen.h \
            $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h     $S/err.h    $S/init.h   $S/strg.h \
            $S/bx86t.r
$S/bx86l.o: $S/bx86l.c  $S/bnull.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/gen.h \
            $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h     $S/err.h    $S/init.h   $S/strg.h \
            $S/bx86l.r
$S/cgr.o:   $S/cgr.c    $S/alist.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/ir.h     $S/cgr.h \
            $S/op.h     $S/gen.h    $S/reg.h    $S/strg.h
$S/dag.o:   $S/dag.c    $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/decl.h   $S/stmt.h \
            $S/gen.h    $S/reg.h    $S/enode.h  $S/err.h    $S/init.h   $S/ir.h     $S/cgr.h \
            $S/op.h     $S/strg.h
$S/decl.o:  $S/decl.c   $S/alist.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/err.h    $S/expr.h \
            $S/in.h     $S/init.h   $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h \
            $S/simp.h   $S/stmt.h   $S/strg.h   $S/decl.h
$S/enode.o: $S/enode.c  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/err.h    $S/op.h \
            $S/simp.h   $S/enode.h
$S/err.o:   $S/err.c    $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/in.h     $S/err.h \
            $S/op.h     $S/strg.h   $S/simp.h   $R/xopt.h
$S/expr.o:  $S/expr.c   $S/decl.h   $S/lex.h    $S/xtoken.h $S/sym.h    $S/alist.h  $S/common.h \
            $S/main.h   $S/ty.h     $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/stmt.h \
            $S/gen.h    $S/reg.h    $S/enode.h  $S/err.h    $S/in.h     $S/ir.h     $S/cgr.h \
            $S/op.h     $S/simp.h   $S/strg.h   $S/expr.h
$S/gen.o:   $S/gen.c    $S/alist.h  $S/cgr.h    $S/dag.h    $S/sym.h    $S/common.h $S/main.h \
            $S/ty.h     $S/lex.h    $S/xtoken.h $S/tree.h   $S/xerror.h $S/cfg.h    $S/op.h \
            $S/ir.h     $S/gen.h    $S/reg.h    $S/strg.h
$S/in.o:    $S/in.c     $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/err.h    $S/in.h
$S/init.o:  $S/init.c   $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/decl.h   $S/stmt.h \
            $S/gen.h    $S/reg.h    $S/enode.h  $S/err.h    $S/expr.h   $S/ir.h     $S/cgr.h \
            $S/op.h     $S/simp.h   $S/init.h
$S/ir.o:    $S/ir.c     $S/ir.h     $S/cfg.h    $S/cgr.h    $S/alist.h  $S/dag.h    $S/sym.h \
            $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/tree.h   $S/xerror.h \
            $S/op.h     $S/gen.h    $S/reg.h
$S/lex.o:   $S/lex.c    $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/err.h    $S/in.h \
            $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/strg.h
$S/main.o:  $S/main.c   $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/decl.h   $S/stmt.h \
            $S/gen.h    $S/reg.h    $S/err.h    $S/in.h     $S/ir.h     $S/cgr.h    $S/op.h \
            $S/strg.h   $S/../version.h
$S/op.o:    $S/op.c     $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h    $S/alist.h  $S/common.h \
            $S/main.h   $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/op.h
$S/reg.o:   $S/reg.c    $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/ir.h     $S/cgr.h \
            $S/op.h     $S/gen.h    $S/reg.h    $S/strg.h
$S/simp.o:  $S/simp.c   $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/enode.h  $S/err.h \
            $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/stmt.h \
            $S/simp.h
$S/stmt.o:  $S/stmt.c   $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/decl.h   $S/stmt.h \
            $S/gen.h    $S/reg.h    $S/enode.h  $S/err.h    $S/expr.h   $S/ir.h     $S/cgr.h \
            $S/op.h     $S/simp.h   $S/strg.h
$S/strg.o:  $S/strg.c   $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/strg.h   $S/err.h
$S/sym.o:   $S/sym.c    $S/alist.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/err.h    $S/ir.h \
            $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/strg.h
$S/tree.o:  $S/tree.c   $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h $S/sym.h \
            $S/alist.h  $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/enode.h  $S/err.h \
            $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/simp.h \
            $S/strg.h
$S/ty.o:    $S/ty.c     $S/alist.h  $S/common.h $S/main.h   $S/ty.h     $S/lex.h    $S/xtoken.h \
            $S/sym.h    $S/dag.h    $S/tree.h   $S/xerror.h $S/cfg.h    $S/decl.h   $S/stmt.h \
            $S/gen.h    $S/reg.h    $S/err.h    $S/ir.h     $S/cgr.h    $S/op.h     $S/strg.h

deps:
	$(MAKE) -C $D/ocelot-$(OV) static

# end of Makefile
