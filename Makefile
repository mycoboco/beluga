#
# Makefile for buliding all
#

SHELL = /bin/sh

SCDIR = ./cpp
BLGDIR = ./src
DRVDIR = ./bcc
P = $(SCDIR)
B = $(BLGDIR)
R = $(DRVDIR)


what:
	-@echo make all clean test

all: sc beluga bcc

clean:
	$(MAKE) -C $P clean
	$(MAKE) -C $B clean
	$(MAKE) -C $R clean

test:
	$(MAKE) -C $P test && $(MAKE) -C $B test && $(MAKE) -C $R test

sc:
	$(MAKE) -C $P all

beluga:
	$(MAKE) -C $B all

bcc:
	$(MAKE) -C $R all

# end of Makefile
