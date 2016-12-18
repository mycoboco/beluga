#
# Makefile for buliding all
#

SHELL = /bin/sh

RM = @rm -f

BLDDIR = ./build
BLGDIR = ./src
DRVDIR = ./bcc
B = $(BLDDIR)
C = $(BLGDIR)
R = $(DRVDIR)


all: $B/beluga $B/bcc

clean:
	cd $C && $(MAKE) clean
	cd $R && $(MAKE) clean
	$(RM) -f $B/beluga $B/bcc $B/xfloat.o

test:
	cd $C && $(MAKE) test

$B/beluga:
	cd $C && $(MAKE) all

$B/bcc:
	cd $R && $(MAKE) all

# end of Makefile
