SUBDIRS = . t

bin_PROGRAMS = pfc

pfc_SOURCES = \
	pfc.c \
	gen.c \
	filter.c \
	fg-util.c \
	fg-iptables.c \
	fg-ipchains.c \
	fg-ipfilter.c \
	fg-cisco.c \
	parser.y \
	scanner.l \
	glue.c \
	resolver.c \
	icmpent.c

pfc_LDADD = @GETOPT_LIBS@

headers = filter.h util.h ast.h resolver.h icmpent.h

mans = filter_backends.7 filter_syntax.5 pfc.8

man_MANS = $(mans)

EXTRA_DIST = HISTORY HONESTY README TODO $(headers) $(mans) pfc.spec

AM_CFLAGS = -D_GNU_SOURCE -g -Wall -Werror
AM_LDFLAGS = -g
AM_YFLAGS = -d
