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


COMPILER = $B/beluga $B/conf.lst
DRIVER = $B/bcc $B/xfloat.o


all: $(COMPILER) $(DRIVER)

clean:
	cd $C && $(MAKE) clean
	cd $R && $(MAKE) clean
	$(RM) -f $(COMPILER) $(DRIVER)

test:
	cd $C && $(MAKE) test

$(COMPILER):
	cd $C && $(MAKE) all

$(DRIVER):
	cd $R && $(MAKE) all

# end of Makefile
