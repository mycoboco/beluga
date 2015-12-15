#
# Makefile for buliding bcc
#

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

RM = rm -f

BLDDIR = ../build
SRCDIR = .
BELDIR = ../src
DEPSDIR = ../deps
HOSTDIR = ./host
B = $(BLDDIR)
S = $(SRCDIR)
L = $(BELDIR)
D = $(DEPSDIR)
H = $(HOSTDIR)

OV = nightly


ALL_CFLAGS = -I$D/ocelot-$(OV)/build/include $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)

.c.o:
	$(CC) -o $@ -c $(CPPFLAGS) $(ALL_CFLAGS) $<


OBJS = $S/bcc.o

DEPS = $D/ocelot-$(OV)/build/lib/libcdsl.a $D/ocelot-$(OV)/build/lib/libcbl.a


all: deps $B/xfloat.o $B/bcc

clean:
	$(RM) $(OBJS) $S/ecgen $S/ec.h
	cd $D/ocelot-$(OV) && $(MAKE) clean

$B/xfloat.o: $S/xfloat.c
	$(CC) -o $@ -c -I$B/include $(CPPFLAGS) $(ALL_CFLAGS) $<

$B/bcc: $(OBJS)
	$(CC) $(ALL_LDFLAGS) -o $B/bcc $(OBJS) $(DEPS)

$S/bcc.o: $S/bcc.c   $S/ec.h     $S/../version.h $S/xopt.h $H/sc.h $H/beluga.h $H/as.h $H/ld.h
$S/ec.h:  $S/ecgen.c $L/xerror.h
	$(CC) -o $S/ecgen $S/ecgen.c
	$S/ecgen > $S/ec.h

deps:
	cd $D/ocelot-$(OV) && $(MAKE) static

# end of Makefile
