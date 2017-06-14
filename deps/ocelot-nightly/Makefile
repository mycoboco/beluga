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

CBLHORG = $(CBLOBJS:.o=.h)
CDSLHORG = $(CDSLOBJS:.o=.h)
CELHORG = $(CELOBJS:.o=.h)
HCPY = $I/cbl/arena.h $I/cbl/assert.h $I/cbl/except.h $I/cbl/memory.h $I/cbl/text.h \
	$I/cdsl/bitv.h $I/cdsl/dlist.h $I/cdsl/dwa.h $I/cdsl/hash.h $I/cdsl/list.h \
	$I/cdsl/set.h $I/cdsl/stack.h $I/cdsl/table.h \
	$I/cel/conf.h $I/cel/opt.h

STATICLIB = $L/libcbl.a $L/libcbld.a $L/libcdsl.a $L/libcel.a
SHAREDLIB = $L/libcbl.so.$M.$N $L/libcbl.so.$M $L/libcbl.so \
	$L/libcbld.so.$M.$N $L/libcbld.so.$M $L/libcbld.so \
	$L/libcdsl.so.$M.$N $L/libcdsl.so.$M $L/libcdsl.so \
	$L/libcel.so.$M.$N $L/libcel.so.$M $L/libcel.so

all: $(HCPY) $(STATICLIB) $(SHAREDLIB)

static: $(HCPY) $(STATICLIB)

clean:
	$(RM) $(CBLOBJS) $(CBLDOBJS) $(CDSLOBJS) $(CELOBJS) $(HCPY) $(STATICLIB) $(SHAREDLIB)

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

$L/libcbl.so.$M: $L/libcbl.so.$M.$N
	cd $L && $(LN) libcbl.so.$M.$N libcbl.so.$M

$L/libcbld.so.$M: $L/libcbld.so.$M.$N
	cd $L && $(LN) libcbld.so.$M.$N libcbld.so.$M

$L/libcdsl.so.$M: $L/libcdsl.so.$M.$N
	cd $L && $(LN) libcdsl.so.$M.$N libcdsl.so.$M

$L/libcel.so.$M: $L/libcel.so.$M.$N
	cd $L && $(LN) libcel.so.$M.$N libcel.so.$M

$L/libcbl.so: $L/libcbl.so.$M
	cd $L && $(LN) libcbl.so.$M libcbl.so

$L/libcbld.so: $L/libcbld.so.$M
	cd $L && $(LN) libcbld.so.$M libcbld.so

$L/libcdsl.so: $L/libcdsl.so.$M
	cd $L && $(LN) libcdsl.so.$M libcdsl.so

$L/libcel.so: $L/libcel.so.$M
	cd $L && $(LN) libcel.so.$M libcel.so

$(HCPY): $I/cbl $I/cdsl $I/cel $(CBLHORG) $(CDSLHORG) $(CELHORG)
	$(CP) $(CBLHORG) $I/cbl/ &&  \
	$(CP) $(CDSLHORG) $I/cdsl/ && \
	$(CP) $(CELHORG) $I/cel/

$I/cbl $I/cdsl $I/cel:
	$(MKDIR) $@

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
