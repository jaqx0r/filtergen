# $Id: Makefile,v 1.6 2002/04/14 15:47:25 matthew Exp $

CC=gcc
CFLAGS=-g -Wall -Werror
LDFLAGS=-g

PROGS=filtergen
BINDIR=/usr/local/sbin

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
	install -m755 $(PROGS) $(BINDIR)
