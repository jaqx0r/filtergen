# $Id: Makefile,v 1.1 2001/09/25 17:22:39 matthew Exp $

include ../fk/Makefile.defs

PROGS=filtergen

LDFLAGS=-lfl

all: $(PROGS)

%.yy.c: %.l
	flex -o$@ $<

filtergen: filtergen.o filter.o filterlex.yy.o fg-iptables.o
	$(CC) -o $@ filtergen.o filter.o filterlex.yy.o fg-iptables.o $(LDFLAGS)

clean:
	rm -f *.o *~ core $(PROGS) *.yy.c

install:
	install -m755 $(BINDIR)
