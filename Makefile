# $Id: Makefile,v 1.9 2002/08/20 13:46:16 matthew Exp $

CC=gcc
CFLAGS=-g -Wall -Werror
LDFLAGS=-g

PROGS=filtergen
PREFIX=$(DESTDIR)/usr
BINDIR=$(PREFIX)/sbin
MANDIR=$(PREFIX)/share/man

LDFLAGS=-lfl

all: $(PROGS)

%.yy.c: %.l
	flex -B -o$@ $<

OBJS=filtergen.o gen.o filter.o fg-util.o fg-iptables.o fg-ipchains.o fg-ipfilter.o fg-cisco.o scanner.o

filtergen: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *~ core $(PROGS) *.yy.c

install:
	install -d $(BINDIR)
	for i in 5 7 8; do \
		install -d $(MANDIR)/man$$i ; \
	done
	install -m755 $(PROGS) $(BINDIR)
	install -m644 *.5 $(MANDIR)/man5
	install -m644 *.7 $(MANDIR)/man7
	install -m644 *.8 $(MANDIR)/man8
