SUBDIRS = . t

bin_PROGRAMS = filtergen

filtergen_SOURCES = \
	filtergen.c \
	gen.c \
	filter.c \
	fg-util.c \
	fg-iptables.c \
	fg-ipchains.c \
	fg-ipfilter.c \
	fg-cisco.c \
	parser.y \
	scanner.l

headers = filter.h util.h ast.h

mans = filtergen.8 filter_backends.7 filter_syntax.5

man_MANS = $(mans)

EXTRA_DIST = HISTORY HONESTY README TODO $(headers) $(mans)

AM_CFLAGS = -D_GNU_SOURCE -g -Wall -Werror
AM_LDFLAGS = -g
AM_YFLAGS = -d