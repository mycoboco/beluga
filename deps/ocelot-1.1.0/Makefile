#
# Makefile for buliding ocelot
#

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

AR = ar ruv
RANLIB = ranlib
CP = cp -f
RM = rm -f
MKDIR = mkdir
CD = cd
GCC = gcc
LN = ln -sf
SHAREDOPT = -shared

BLDDIR = build
SRCDIR = src
INCDIR = $B/include
LIBDIR = $B/lib
B = $(BLDDIR)
S = $(SRCDIR)
I = $(INCDIR)
L = $(LIBDIR)

M = 1
N = 1


ALL_CFLAGS = -I$I -fPIC $(CFLAGS)

.c.o:
	$(CC) -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<


CBLOBJS = $S/cbl/arena.o $S/cbl/assert.o $S/cbl/except.o $S/cbl/memory.o $S/cbl/text.o
CBLDOBJS = $S/cbl/arena.o $S/cbl/assert.o $S/cbl/except.o $S/cbl/memoryd.o $S/cbl/text.o
CDSLOBJS = $S/cdsl/bitv.o $S/cdsl/dlist.o $S/cdsl/dwa.o $S/cdsl/hash.o $S/cdsl/list.o \
	$S/cdsl/set.o $S/cdsl/stack.o $S/cdsl/table.o
CELOBJS = $S/cel/conf.o $S/cel/opt.o

CBLHORIG = $(CBLOBJS:.o=.h)
CDSLHORIG = $(CDSLOBJS:.o=.h)
CELHORIG = $(CELOBJS:.o=.h)

CBLHCOPY = $I/cbl/arena.h $I/cbl/assert.h $I/cbl/except.h $I/cbl/memory.h $I/cbl/text.h
CDSLHCOPY = $I/cdsl/bitv.h $I/cdsl/dlist.h $I/cdsl/dwa.h $I/cdsl/hash.h $I/cdsl/list.h \
	$I/cdsl/set.h $I/cdsl/stack.h $I/cdsl/table.h
CELHCOPY = $I/cel/conf.h $I/cel/opt.h


all: cbl cdsl cel

cbl: $(CBLHCOPY) $L/libcbl.a $L/libcbl.so.$M.$N $L/libcbl.so $L/libcbld.a $L/libcbld.so.$M.$N \
	$L/libcbld.so

cdsl: $(CBLHCOPY) $(CDSLHCOPY) $L/libcdsl.a $L/libcdsl.so.$M.$N $L/libcdsl.so

cel: $(CBLHCOPY) $(CDSLHCOPY) $(CELHCOPY) $L/libcel.a $L/libcel.so.$M.$N $L/libcel.so

static: $(CBLHCOPY) $(CDSLHCOPY) $(CELHCOPY) $L/libcbl.a $L/libcbld.a $L/libcdsl.a $L/libcel.a

clean:
	$(RM) $(CBLOBJS) $(CBLDOBJS) $(CDSLOBJS) $(CELOBJS)


$L/libcbl.a: $(CBLOBJS)
	$(AR) $@ $(CBLOBJS); $(RANLIB) $@ || true

$L/libcbld.a: $(CBLDOBJS)
	$(AR) $@ $(CBLDOBJS); $(RANLIB) $@ || true

$L/libcdsl.a: $(CDSLOBJS)
	$(AR) $@ $(CDSLOBJS); $(RANLIB) $@ || true

$L/libcel.a: $(CELOBJS)
	$(AR) $@ $(CELOBJS); $(RANLIB) $@ || true

$L/libcbl.so.$M.$N: $(CBLOBJS)
	$(GCC) $(CFLAGS) $(SHAREDOPT) -Wl,-soname,libcbl.so.$M -o $@ $(CBLOBJS)

$L/libcbld.so.$M.$N: $(CBLDOBJS)
	$(GCC) $(CFLAGS) $(SHAREDOPT) -Wl,-soname,libcbld.so.$M -o $@ $(CBLDOBJS)

$L/libcdsl.so.$M.$N: $(CDSLOBJS)
	$(GCC) $(CFLAGS) $(SHAREDOPT) -Wl,-soname,libcdsl.so.$M -o $@ $(CDSLOBJS)

$L/libcel.so.$M.$N: $(CELOBJS)
	$(GCC) $(CFLAGS) $(SHAREDOPT) -Wl,-soname,libcel.so.$M -o $@ $(CELOBJS)

$L/libcbl.so: $L/libcbl.so.$M.$N
	$(CD) $L && \
	$(LN) libcbl.so.$M.$N libcbl.so.$M && \
	$(LN) libcbl.so.$M libcbl.so

$L/libcbld.so: $L/libcbld.so.$M.$N
	$(CD) $L && \
	$(LN) libcbld.so.$M.$N libcbld.so.$M && \
	$(LN) libcbld.so.$M libcbld.so

$L/libcdsl.so: $L/libcdsl.so.$M.$N
	$(CD) $L && \
	$(LN) libcdsl.so.$M.$N libcdsl.so.$M && \
	$(LN) libcdsl.so.$M libcdsl.so

$L/libcel.so: $L/libcel.so.$M.$N
	$(CD) $L && \
	$(LN) libcel.so.$M.$N libcel.so.$M && \
	$(LN) libcel.so.$M libcel.so

$(CBLHCOPY): $I/cbl $(CBLHORIG)
	$(CP) $(CBLHORIG) $I/cbl/

$(CDSLHCOPY): $I/cdsl $(CDSLHORIG)
	$(CP) $(CDSLHORIG) $I/cdsl/

$(CELHCOPY): $I/cel $(CELHORIG)
	$(CP) $(CELHORIG) $I/cel/

$I/cbl:
	$(MKDIR) $I/cbl

$I/cdsl:
	$(MKDIR) $I/cdsl

$I/cel:
	$(MKDIR) $I/cel

$S/cbl/arena.o:   $S/cbl/arena.c   $S/cbl/arena.h  $S/cbl/assert.h $S/cbl/except.h
$S/cbl/assert.o:  $S/cbl/assert.c  $S/cbl/assert.h $S/cbl/except.h
$S/cbl/except.o:  $S/cbl/except.c  $S/cbl/except.h $S/cbl/assert.h
$S/cbl/memory.o:  $S/cbl/memory.c  $S/cbl/memory.h $S/cbl/assert.h $S/cbl/except.h
$S/cbl/memoryd.o: $S/cbl/memoryd.c $S/cbl/memory.h $S/cbl/assert.h $S/cbl/except.h
$S/cbl/text.o:    $S/cbl/text.c    $S/cbl/text.h   $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h

$S/cdsl/bitv.o:  $S/cdsl/bitv.c  $S/cdsl/bitv.h  $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h
$S/cdsl/dlist.o: $S/cdsl/dlist.c $S/cdsl/dlist.h $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h
$S/cdsl/dwa.o:   $S/cdsl/dwa.c   $S/cdsl/dwa.h   $S/cbl/assert.h $S/cbl/except.h
$S/cdsl/hash.o:  $S/cdsl/hash.c  $S/cdsl/hash.h  $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h
$S/cdsl/list.o:  $S/cdsl/list.c  $S/cdsl/list.h	 $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h
$S/cdsl/set.o:   $S/cdsl/set.c   $S/cdsl/set.h   $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h
$S/cdsl/stack.o: $S/cdsl/stack.c $S/cdsl/stack.h $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h
$S/cdsl/table.o: $S/cdsl/table.c $S/cdsl/table.h $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h

$S/cel/conf.o: $S/cel/conf.c $S/cel/conf.h $S/cbl/assert.h $S/cbl/except.h $S/cbl/memory.h \
	$S/cdsl/hash.h $S/cdsl/table.h
$S/cel/opt.o:  $S/cel/opt.c  $S/cel/opt.h

# end of Makefile
