#
# Makefile for buliding all
#

SHELL = /bin/sh

BLDDIR = ./build
SCDIR = ./cpp
BLGDIR = ./src
DRVDIR = ./bcc
B = $(BLDDIR)
P = $(SCDIR)
C = $(BLGDIR)
R = $(DRVDIR)


what:
	-@echo make all clean test

all: $B/sc $B/beluga $B/bcc

clean:
	$(MAKE) -C $P clean
	$(MAKE) -C $C clean
	$(MAKE) -C $R clean

test:
	$(MAKE) -C $P test && $(MAKE) -C $C test

$B/sc:
	$(MAKE) -C $P all

$B/beluga:
	$(MAKE) -C $C all

$B/bcc:
	$(MAKE) -C $R all

# end of Makefile
