#
# Makefile
#

### VARIABLES #################################################################
EXEC	 = client server
SRCDIR 	 = src
BUILDDIR = build
DOCDIR   = docs

### RULES #####################################################################
.PHONY: all build

all: build 

build:
	make -C $(SRCDIR) $(EXEC)
	cd $(SRCDIR) && mv $(EXEC) ../$(BUILDDIR)

clean:
	make -C $(SRCDIR) clean
	cd $(BUILDDIR) && rm $(EXEC)
