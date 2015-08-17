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
	mkdir $(BUILDDIR)
	 cd $(SRCDIR) && mv $(EXEC) ../$(BUILDDIR)

clean:
	make -C $(SRCDIR) clean
	mkdir $(BUILDDIR)
	 cd $(BUILDDIR) && rm $(EXEC)
