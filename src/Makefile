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

OV = nightly


ALL_CFLAGS = -I$S -I$L -I$C -I$D/ocelot-$(OV)/build/include $(CFLAGS)
ALL_LDFLAGS = -lm $(LDFLAGS)

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

CONF = $B/conf.lst
TEST = $T/cpp/*.new $T/mcpp/*.new $T/diag/*.new $T/asm/*.s


all: $B/beluga $(CONF)

clean:
	$(RM) $(OBJS) $(GPROF) $(GCOV) $(CONF) $(TEST)
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

$B/beluga: $(OBJS) $(DEPS)
	$(CC) $(ALL_LDFLAGS) -o $@ $(OBJS) $(DEPS)

$L/cond.o: $(DEPS)     $L/cond.c   $L/common.h $L/main.h   $L/strg.h  $L/err.h  $L/lmap.h \
           $L/lex.h    $L/xtoken.h $S/sym.h    $S/alist.h  $S/dag.h   $S/tree.h $S/ty.h \
           $S/cfg.h    $L/xerror.h $L/cond.h
$L/err.o:  $(DEPS)     $L/err.c    $S/clx.h    $L/common.h $L/strg.h  $L/main.h $L/lex.h \
           $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h $S/dag.h  $S/tree.h \
           $S/ty.h     $S/cfg.h    $L/cond.h   $L/in.h     $S/simp.h  $L/err.h  $L/xerror.h
$L/expr.o: $(DEPS)     $L/expr.c   $S/clx.h    $L/common.h $L/main.h  $L/strg.h $L/lex.h \
           $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h $S/dag.h  $S/tree.h \
           $S/ty.h     $S/cfg.h    $L/err.h    $L/xerror.h $L/mcr.h   $L/expr.h
$L/in.o:   $(DEPS)     $L/in.c     $L/common.h $L/main.h   $L/strg.h  $L/err.h  $L/lmap.h \
           $L/lex.h    $L/xtoken.h $S/sym.h    $S/alist.h  $S/dag.h   $S/tree.h $S/ty.h \
           $S/cfg.h    $L/xerror.h $L/inc.h    $L/cond.h   $L/util.h  $L/in.h
$L/inc.o:  $(DEPS)     $L/inc.c    $L/common.h $L/main.h   $L/strg.h  $L/cond.h $L/lmap.h \
           $L/lex.h    $L/xtoken.h $L/err.h    $S/sym.h    $S/alist.h $S/dag.h  $S/tree.h \
           $S/ty.h     $S/cfg.h    $L/xerror.h $L/in.h     $L/mg.h    $L/util.h $L/inc.h
$L/lex.o:  $(DEPS)     $L/lex.c    $L/common.h $L/main.h   $L/strg.h  $L/err.h  $L/lmap.h \
           $L/lex.h    $L/xtoken.h $S/sym.h    $S/alist.h  $S/dag.h   $S/tree.h $S/ty.h \
           $S/cfg.h    $L/xerror.h $L/in.h
$L/lmap.o: $(DEPS)     $L/lmap.c   $L/common.h $L/main.h   $L/strg.h  $L/in.h   $L/lex.h \
           $L/lmap.h   $L/xtoken.h
$L/lst.o:  $(DEPS)     $L/lst.c    $L/in.h     $L/common.h $L/main.h  $L/strg.h $L/lex.h \
           $L/lmap.h   $L/xtoken.h $L/mcr.h    $L/proc.h   $L/lst.h
$L/main.o: $(DEPS)     $L/main.c   $S/clx.h    $L/common.h $L/main.h  $L/strg.h $L/lex.h \
           $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h $S/dag.h  $S/tree.h \
           $S/ty.h     $S/cfg.h    $C/cpp.h    $S/decl.h   $S/stmt.h  $S/gen.h  $S/reg.h \
           $L/err.h    $L/xerror.h $L/in.h     $L/inc.h    $L/cond.h  $S/ir.h   $S/cgr.h \
           $S/op.h     $L/mcr.h    $S/ty.h     $L/util.h   ../version.h
$L/mcr.o:  $(DEPS)     $L/mcr.c    $L/common.h $L/main.h   $L/strg.h  $L/err.h  $L/lmap.h \
           $L/lex.h    $L/xtoken.h $S/sym.h    $S/alist.h  $S/dag.h   $S/tree.h $S/ty.h \
           $S/cfg.h    $L/xerror.h $L/in.h     $L/lst.h    $L/util.h  $L/mcr.h
$L/mg.o:   $(DEPS)     $L/mg.c     $L/common.h $L/main.h   $L/strg.h  $L/mcr.h  $L/lex.h \
           $L/lmap.h   $L/xtoken.h $L/mg.h
$L/prgm.o: $(DEPS)     $L/prgm.c   $L/lex.h    $L/common.h $L/main.h  $L/strg.h $L/lmap.h \
           $L/xtoken.h $L/lst.h    $L/mg.h     $L/util.h   $L/prgm.h
$L/proc.o: $(DEPS)     $L/proc.c   $L/common.h $L/main.h   $L/strg.h  $L/cond.h $L/lmap.h \
           $L/lex.h    $L/xtoken.h $L/err.h    $S/sym.h    $S/alist.h $S/dag.h  $S/tree.h \
           $S/ty.h     $S/cfg.h    $L/xerror.h $L/expr.h   $L/in.h    $L/inc.h  $L/lst.h \
           $L/mcr.h    $L/mg.h     $L/prgm.h   $L/util.h   $L/proc.h
$L/strg.o: $(DEPS)     $L/strg.c   $L/common.h $L/main.h   $L/strg.h
$L/util.o: $(DEPS)     $L/util.c   $L/util.h

$C/cpp.o:  $(DEPS)  $C/cpp.c  $L/common.h $L/main.h $L/strg.h $L/lex.h $L/lmap.h $L/xtoken.h \
           $L/lst.h $L/proc.h $C/cpp.h

$S/alist.o: $(DEPS)     $S/alist.c  $S/alist.h  $L/common.h $L/main.h   $L/strg.h
$S/cgr.o:   $(DEPS)     $S/cgr.c    $S/alist.h  $L/common.h $L/main.h   $L/strg.h $S/ir.h \
            $S/cfg.h    $S/cgr.h    $S/dag.h    $S/sym.h    $L/lmap.h   $L/lex.h  $L/xtoken.h \
            $S/tree.h   $S/ty.h     $S/op.h     $S/gen.h    $S/reg.h
$S/clx.o:   $(DEPS)     $S/clx.c    $L/common.h $L/main.h   $L/strg.h   $L/err.h  $L/lmap.h \
            $L/lex.h    $L/xtoken.h $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h \
            $S/cfg.h    $L/xerror.h $S/ir.h     $S/cgr.h    $S/op.h     $S/gen.h  $S/reg.h \
            $L/lst.h    $L/proc.h   $S/clx.h
$S/dag.o:   $(DEPS)     $S/dag.c    $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h  $S/enode.h \
            $L/err.h    $L/xerror.h $S/init.h   $S/ir.h     $S/cgr.h    $S/op.h
$S/decl.o:  $(DEPS)     $S/decl.c   $S/alist.h  $L/common.h $L/main.h   $L/strg.h $S/clx.h \
            $L/lex.h    $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $L/err.h    $L/xerror.h $S/expr.h   $S/init.h $S/ir.h \
            $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/sset.h   $S/simp.h $S/stmt.h \
            $S/decl.h
$S/enode.o: $(DEPS)     $S/enode.c  $L/lmap.h   $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/xtoken.h $L/err.h    $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/ty.h \
            $S/cfg.h    $L/xerror.h $S/op.h     $S/simp.h   $S/enode.h
$S/expr.o:  $(DEPS)     $S/expr.c   $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h  $S/enode.h \
            $L/err.h    $L/xerror.h $S/ir.h     $S/cgr.h    $S/op.h     $S/sset.h $S/simp.h \
            $S/expr.h
$S/gen.o:   $(DEPS)     $S/gen.c    $S/alist.h  $L/common.h $L/main.h   $L/strg.h $S/cgr.h \
            $S/dag.h    $S/sym.h    $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h $S/ty.h \
            $S/cfg.h    $S/op.h     $S/ir.h     $S/gen.h    $S/reg.h
$S/init.o:  $(DEPS)     $S/init.c   $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/enode.h  $L/err.h    $L/xerror.h $S/expr.h $S/ir.h \
            $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/sset.h   $S/simp.h $S/init.h
$S/ir.o:    $(DEPS)     $S/ir.c     $S/ir.h     $S/cfg.h    $L/common.h $L/main.h $L/strg.h \
            $S/cgr.h    $S/alist.h  $S/dag.h    $S/sym.h    $L/lmap.h   $L/lex.h  $L/xtoken.h \
            $S/tree.h   $S/ty.h     $S/op.h     $S/gen.h    $S/reg.h
$S/op.o:    $(DEPS)     $S/op.c     $S/ty.h     $L/lex.h    $L/common.h $L/main.h $L/strg.h \
            $L/lmap.h   $L/xtoken.h $S/sym.h    $S/alist.h  $S/dag.h    $S/tree.h $S/cfg.h \
            $S/op.h
$S/reg.o:   $(DEPS)     $S/reg.c    $L/common.h $L/main.h   $L/strg.h   $S/dag.h  $S/sym.h \
            $S/alist.h  $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h   $S/ty.h   $S/cfg.h \
            $S/gen.h    $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h
$S/simp.o:  $(DEPS)     $S/simp.c   $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/enode.h  $L/err.h    $L/xerror.h $S/expr.h $S/ir.h \
            $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/stmt.h   $S/simp.h
$S/sset.o:  $(DEPS)     $S/sset.c   $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $L/err.h    $L/xerror.h
$S/stmt.o:  $(DEPS)     $S/stmt.c   $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h  $S/enode.h \
            $L/err.h    $L/xerror.h $S/expr.h   $S/ir.h     $S/cgr.h    $S/op.h   $S/simp.h \
            $S/sset.h
$S/sym.o:   $(DEPS)     $S/sym.c    $S/alist.h  $L/common.h $L/main.h   $L/strg.h $S/clx.h \
            $L/lex.h    $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $L/err.h    $L/xerror.h $S/ir.h     $S/cgr.h  $S/op.h \
            $S/gen.h    $S/reg.h    $L/util.h
$S/tree.o:  $(DEPS)     $S/tree.c   $S/clx.h    $L/common.h $L/main.h   $L/strg.h $L/lex.h \
            $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/alist.h  $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/enode.h  $L/err.h    $L/xerror.h $S/expr.h $S/ir.h \
            $S/cgr.h    $S/op.h     $S/gen.h    $S/reg.h    $S/simp.h   $S/sset.h $L/xtoken.h
$S/ty.o:    $(DEPS)     $S/ty.c     $S/alist.h  $L/common.h $L/main.h   $L/strg.h $S/clx.h \
            $L/lex.h    $L/lmap.h   $L/xtoken.h $L/lst.h    $S/sym.h    $S/dag.h  $S/tree.h \
            $S/ty.h     $S/cfg.h    $S/decl.h   $S/stmt.h   $S/gen.h    $S/reg.h  $L/err.h \
            $L/xerror.h $S/ir.h     $S/cgr.h    $S/op.h
$S/bnull.o: $(DEPS)     $S/bnull.c  $S/bnull.h  $L/common.h $L/main.h   $L/strg.h $S/dag.h \
            $S/sym.h    $S/alist.h  $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h $S/ty.h \
            $S/cfg.h    $S/gen.h    $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h
$S/bx86t.o: $(DEPS)     $S/bx86t.c  $S/bnull.h  $L/common.h $L/main.h   $L/strg.h $S/dag.h \
            $S/sym.h    $S/alist.h  $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h $S/ty.h \
            $S/cfg.h    $S/gen.h    $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h   $S/clx.h \
            $L/lst.h    $L/err.h    $L/xerror.h $S/init.h   $S/bx86t.r
$S/bx86l.o: $(DEPS)     $S/bx86l.c  $S/bnull.h  $L/common.h $L/main.h   $L/strg.h $S/dag.h \
            $S/sym.h    $S/alist.h  $L/lmap.h   $L/lex.h    $L/xtoken.h $S/tree.h $S/ty.h \
            $S/cfg.h    $S/gen.h    $S/reg.h    $S/ir.h     $S/cgr.h    $S/op.h   $S/clx.h \
            $L/lst.h    $L/err.h    $L/xerror.h $S/init.h   $S/bx86l.r

$(DEPS):
	$(MAKE) -C $D/ocelot-$(OV) static

$B/conf.lst: $R/conf.c
	$(CC) -o $@ -E $(CPPFLAGS) $(ALL_CFLAGS) $?

# end of Makefile
