# $Id: Makefile,v 1.7 2002/04/28 22:18:10 matthew Exp $

CC=gcc
CFLAGS=-g -Wall -Werror
LDFLAGS=-g

PROGS=filtergen
PREFIX=/usr/local
BINDIR=$(PREFIX)/sbin
MANDIR=$(PREFIX)/share/man

LDFLAGS=-lfl

all: $(PROGS)

%.yy.c: %.l
	flex -B -o$@ $<

OBJS=filtergen.o gen.o filter.o filterlex.yy.o fg-util.o fg-iptables.o fg-ipchains.o fg-ipfilter.o fg-cisco.o

filtergen: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *~ core $(PROGS) *.yy.c

install:
	install -d $(BINDIR) $(MANDIR)/man{5,8}
	install -m755 $(PROGS) $(BINDIR)
	install -m755 *.5 $(MANDIR)/man5
	install -m755 *.8 $(MANDIR)/man8
