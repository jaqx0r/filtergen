# $Id: Makefile,v 1.4 2001/10/03 19:47:17 matthew Exp $

CC=gcc
CFLAGS=-g -Wall -Werror
LDFLAGS=-g

PROGS=filtergen

LDFLAGS=-lfl

all: $(PROGS)

%.yy.c: %.l
	flex -o$@ $<

OBJS=filtergen.o gen.o filter.o filterlex.yy.o fg-util.o fg-iptables.o fg-ipchains.o fg-cisco.o

filtergen: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *~ core $(PROGS) *.yy.c

install:
	install -m755 $(BINDIR)
