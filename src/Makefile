#
# Makefile for buliding beluga
#

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

RM = rm -f
RMR = rm -rf
NODE = node

BLDDIR = ../build
SRCDIR = .
LIBDIR = ../lib
CPPDIR = ../cpp
DRVDIR = ../bcc
PROFDIR = ../prof
TESTDIR = ../tst
DEPSDIR = ../deps
B = $(BLDDIR)
S = $(SRCDIR)
L = $(LIBDIR)
C = $(CPPDIR)
R = $(DRVDIR)
P = $(PROFDIR)
T = $(TESTDIR)
D = $(DEPSDIR)

OV = 1.1.0


ALL_CFLAGS = -I$S -I$L -I$C -I$D/ocelot-$(OV)/build/include $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)

.c.o:
	$(CC) -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<


OBJS = $L/cond.o $L/err.o $L/expr.o $L/in.o $L/inc.o $L/lex.o $L/lmap.o $L/lst.o $L/main.o \
       $L/mcr.o $L/mg.o $L/prgm.o $L/proc.o $L/strg.o $L/util.o $C/cpp.o \
       $S/alist.o $S/cgr.o $S/clx.o $S/dag.o $S/decl.o $S/enode.o $S/expr.o $S/gen.o $S/init.o \
       $S/ir.o $S/op.o $S/reg.o $S/simp.o $S/sset.o $S/stmt.o $S/sym.o $S/tree.o $S/ty.o \
       $S/bnull.o $S/bx86t.o $S/bx86l.o

DEPS = $D/ocelot-$(OV)/build/lib/libcel.a $D/ocelot-$(OV)/build/lib/libcdsl.a \
       $D/ocelot-$(OV)/build/lib/libcbl.a

GPROF = $P/gmon.out $P/gmon.out.* $P/gmon.sum
GCOV = $S/*.gcno $S/*.gcda *.gcov

TEST = $T/cpp/*.new $T/mcpp/*.new $T/diag/*.new $T/asm/*.s


all: deps $B/beluga

clean:
	$(RM) $(OBJS) $(GPROF) $(GCOV) $(TEST)
	$(MAKE) -C $D/ocelot-$(OV) clean

test:
	$(NODE) $T/run.js $T/cpp
	$(NODE) $T/run.js $T/mcpp
	$(NODE) $T/run.js $T/diag
	$(NODE) $T/run.js $T/asm

gprof:
	$(MAKE) clean
	CFLAGS="$(CFLAGS) -g -pg" LDFLAGS="$(LDFLAGS) -pg" $(MAKE) all

gcov:
	$(MAKE) clean
	CFLAGS="$(CFLAGS) -fprofile-arcs -ftest-coverage" \
		LDFLAGS="$(LDFLAGS) -fprofile-arcs -ftest-coverage" $(MAKE) all

$B/beluga: $(OBJS)
	$(CC) $(ALL_LDFLAGS) -o $B/beluga $(OBJS) $(DEPS)

$L/cond.o: $L/cond.c   $L/common.h $L/main.h   $L/err.h    $L/lmap.h $L/lex.h    $L/xtoken.h \
           $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h   $S/cfg.h    $L/xerror.h \
           $L/cond.h
$L/err.o:  $L/err.c    $S/clx.h    $L/common.h $L/main.h   $L/lex.h  $L/lmap.h   $L/xtoken.h \
           $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h     $S/cfg.h \
           $L/cond.h   $L/in.h     $S/simp.h   $L/strg.h   $L/err.h  $L/xerror.h
$L/expr.o: $L/expr.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h  $L/lmap.h   $L/xtoken.h \
           $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h     $S/cfg.h \
           $L/err.h    $L/xerror.h $L/mcr.h    $L/strg.h   $L/expr.h
$L/in.o:   $L/in.c     $L/common.h $L/main.h   $L/err.h    $L/lmap.h $L/lex.h    $L/xtoken.h \
           $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h   $S/cfg.h    $L/xerror.h \
           $L/inc.h    $L/cond.h   $L/util.h   $L/in.h
$L/inc.o:  $L/inc.c    $L/common.h $L/main.h   $L/cond.h   $L/lmap.h $L/lex.h    $L/xtoken.h \
           $L/err.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h     $S/cfg.h \
           $L/xerror.h $L/in.h     $L/mg.h     $L/strg.h   $L/util.h $L/inc.h
$L/lex.o:  $L/lex.c    $L/common.h $L/main.h   $L/err.h    $L/lmap.h $L/lex.h    $L/xtoken.h \
           $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h   $S/cfg.h    $L/xerror.h \
           $L/in.h     $L/strg.h
$L/lmap.o: $L/lmap.c   $L/common.h $L/main.h   $L/in.h     $L/lex.h  $L/lmap.h   $L/xtoken.h \
           $L/strg.h
$L/lst.o:  $L/lst.c    $L/in.h     $L/common.h $L/main.h   $L/lex.h  $L/lmap.h   $L/xtoken.h \
           $L/mcr.h    $L/proc.h   $L/strg.h   $L/lst.h
$L/main.o: $L/main.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h  $L/lmap.h   $L/xtoken.h \
           $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h     $S/cfg.h \
           $C/cpp.h    $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h  $L/err.h    $L/xerror.h \
           $L/in.h     $L/inc.h    $L/cond.h   $S/ir.h     $S/cgr.h  $S/op.h     $L/mcr.h \
           $L/strg.h   $S/ty.h     $L/util.h   ../version.h
$L/mcr.o:  $L/mcr.c    $L/common.h $L/main.h   $L/err.h    $L/lmap.h $L/lex.h    $L/xtoken.h \
           $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h   $S/cfg.h    $L/xerror.h \
           $L/in.h     $L/lst.h    $L/strg.h   $L/util.h   $L/mcr.h
$L/mg.o:   $L/mg.c     $L/common.h $L/main.h   $L/mcr.h    $L/lex.h  $L/lmap.h   $L/xtoken.h \
           $L/strg.h   $L/mg.h
$L/prgm.o: $L/prgm.c   $L/lex.h    $L/common.h $L/main.h   $L/lmap.h $L/xtoken.h $L/lst.h \
           $L/mg.h     $L/util.h   $L/prgm.h
$L/proc.o: $L/proc.c   $L/common.h $L/main.h   $L/cond.h   $L/lmap.h $L/lex.h    $L/xtoken.h \
           $L/err.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h     $S/cfg.h \
           $L/xerror.h $L/expr.h   $L/in.h     $L/inc.h    $L/lst.h  $L/mcr.h    $L/mg.h \
           $L/prgm.h   $L/strg.h   $L/util.h   $L/proc.h
$L/strg.o: $L/strg.c   $L/common.h $L/main.h   $L/strg.h
$L/util.o: $L/util.c   $L/util.h

$C/cpp.o:  $C/cpp.c  $L/common.h $L/main.h $L/lex.h $L/lmap.h $L/xtoken.h $L/lst.h $L/proc.h \
           $L/strg.h $C/cpp.h

$S/alist.o: $S/alist.c  $S/alist.h  $L/common.h $L/main.h
$S/cgr.o:   $S/cgr.c    $S/alist.h  $L/common.h $L/main.h   $S/ir.h     $S/cfg.h    $S/cgr.h \
            $S/dag.h    $S/sym.h    $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h  \
            $S/op.h     $S/gen.h    $S/reg.h    $L/strg.h
$S/clx.o:   $S/clx.c    $L/common.h $L/main.h   $L/err.h    $L/lmap.h   $L/lex.h    $L/xtoken.h \
            $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h    $L/xerror.h \
            $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $L/lst.h    $L/proc.h \
            $L/strg.h   $S/clx.h
$S/dag.o:   $S/dag.c    $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h    $S/enode.h  $L/err.h    $L/xerror.h \
            $S/init.h   $S/ir.h     $S/cgr.h    $S/op.h     $L/strg.h
$S/decl.o:  $S/decl.c   $S/alist.h  $L/common.h $L/main.h   $S/clx.h    $L/lex.h    $L/lmap.h \
            $L/xtoken.h $L/lst.h    $S/sym.h    $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $L/err.h    $L/xerror.h $S/expr.h   $S/init.h   $S/ir.h     $S/cgr.h    $S/op.h \
            $S/gen.h    $S/reg.h    $S/sset.h   $S/simp.h   $S/stmt.h   $L/strg.h   $S/decl.h
$S/enode.o: $S/enode.c  $L/lmap.h   $L/common.h $L/main.h   $L/lex.h    $L/xtoken.h $L/err.h \
            $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h    $L/xerror.h \
            $S/op.h     $S/simp.h   $S/enode.h
$S/expr.o:  $S/expr.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h    $S/enode.h  $L/err.h    $L/xerror.h \
            $S/ir.h     $S/cgr.h    $S/op.h     $S/sset.h   $S/simp.h   $L/strg.h   $S/expr.h
$S/gen.o:   $S/gen.c    $S/alist.h  $L/common.h $L/main.h   $S/cgr.h    $S/dag.h    $S/sym.h \
            $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h     $S/cfg.h    $S/op.h \
            $S/ir.h     $S/gen.h    $S/reg.h    $L/strg.h
$S/init.o:  $S/init.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/enode.h  $L/err.h    $L/xerror.h $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h \
            $S/gen.h    $S/reg.h    $S/sset.h   $S/simp.h   $S/init.h
$S/ir.o:    $S/ir.c     $S/ir.h     $S/cfg.h    $S/cgr.h    $S/alist.h  $L/common.h $L/main.h \
            $S/dag.h    $S/sym.h    $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h \
            $S/op.h     $S/gen.h    $S/reg.h
$S/op.o:    $S/op.c     $S/ty.h     $L/lex.h    $L/common.h $L/main.h   $L/lmap.h   $L/xtoken.h \
            $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/cfg.h    $S/op.h
$S/reg.o:   $S/reg.c    $L/common.h $L/main.h   $S/dag.h    $S/sym.h    $S/alist.h  $L/lmap.h \
            $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h     $S/cfg.h    $S/gen.h    $S/reg.h\
            $S/ir.h     $S/cgr.h    $S/op.h     $L/strg.h
$S/simp.o:  $S/simp.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/enode.h  $L/err.h    $L/xerror.h $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h \
            $S/gen.h    $S/reg.h    $S/stmt.h   $S/simp.h
$S/sset.o:  $S/sset.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $L/err.h    $L/xerror.h
$S/stmt.o:  $S/stmt.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h    $S/enode.h  $L/err.h    $L/xerror.h \
            $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h     $S/simp.h   $S/sset.h   $L/strg.h
$S/sym.o:   $S/sym.c    $S/alist.h  $L/common.h $L/main.h   $S/clx.h    $L/lex.h    $L/lmap.h \
            $L/xtoken.h $L/lst.h    $S/sym.h    $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $L/err.h    $L/xerror.h $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h \
            $L/strg.h   $L/util.h
$S/tree.o:  $S/tree.c   $S/clx.h    $L/common.h $L/main.h   $L/lex.h    $L/lmap.h   $L/xtoken.h \
            $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/enode.h  $L/err.h    $L/xerror.h $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h \
            $S/gen.h    $S/reg.h    $S/simp.h   $S/sset.h   $L/strg.h   $L/xtoken.h
$S/ty.o:    $S/ty.c     $S/alist.h  $L/common.h $L/main.h   $S/clx.h    $L/lex.h    $L/lmap.h \
            $L/xtoken.h $L/lst.h    $S/sym.h    $S/dag.h    $S/tree.h   $S/ty.h     $S/cfg.h \
            $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h    $L/err.h    $L/xerror.h $S/ir.h \
            $S/cgr.h    $S/op.h     $L/strg.h
$S/bnull.o: $S/bnull.c  $S/bnull.h  $L/common.h $L/main.h   $S/dag.h    $S/sym.h    $S/alist.h \
            $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h     $S/cfg.h    $S/gen.h \
            $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h
$S/bx86t.o: $S/bx86t.c  $S/bnull.h  $L/common.h $L/main.h   $S/dag.h    $S/sym.h    $S/alist.h \
            $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h     $S/cfg.h    $S/gen.h \
            $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h     $S/clx.h    $L/lst.h    $L/err.h \
            $L/xerror.h $S/init.h   $L/strg.h   $S/bx86t.r
$S/bx86l.o: $S/bx86l.c  $S/bnull.h  $L/common.h $L/main.h   $S/dag.h    $S/sym.h    $S/alist.h \
            $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h     $S/cfg.h    $S/gen.h \
            $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h     $S/clx.h    $L/lst.h    $L/err.h \
            $L/xerror.h $S/init.h   $L/strg.h   $S/bx86l.r

deps:
	$(MAKE) -C $D/ocelot-$(OV) static

# end of Makefile
