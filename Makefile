# $Id: Makefile,v 1.5 2001/10/04 14:02:43 matthew Exp $

CC=gcc
CFLAGS=-g -Wall -Werror
LDFLAGS=-g

PROGS=filtergen

LDFLAGS=-lfl

all: $(PROGS)

%.yy.c: %.l
	flex -o$@ $<

OBJS=filtergen.o gen.o filter.o filterlex.yy.o fg-util.o fg-iptables.o fg-ipchains.o fg-ipfilter.o fg-cisco.o

filtergen: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *~ core $(PROGS) *.yy.c

install:
	install -m755 $(BINDIR)
