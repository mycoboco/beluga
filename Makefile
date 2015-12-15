#
# Makefile for buliding all
#

SHELL = /bin/sh

RM = @rm -f

BLDDIR = ./build
SCDIR = ./cpp
BLGDIR = ./src
DRVDIR = ./bcc
B = $(BLDDIR)
P = $(SCDIR)
C = $(BLGDIR)
R = $(DRVDIR)


all: $B/sc $B/beluga $B/bcc

clean:
	cd $P && $(MAKE) clean
	cd $C && $(MAKE) clean
	cd $R && $(MAKE) clean
	$(RM) -f $B/sc $B/beluga $B/bcc $B/xfloat.o

test:
	cd $P && $(MAKE) test
	cd $C && $(MAKE) test

$B/sc:
	cd $P && $(MAKE) all

$B/beluga:
	cd $C && $(MAKE) all

$B/bcc:
	cd $R && $(MAKE) all

# end of Makefile
