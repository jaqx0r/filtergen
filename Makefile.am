bin_PROGRAMS = filtergen

filtergen_SOURCES = \
	filtergen.c \
	gen.c \
	filter.c \
	filterlex.l \
	fg-util.c \
	fg-iptables.c \
	fg-ipchains.c \
	fg-ipfilter.c \
	fg-cisco.c

man_MANS = filtergen.8 filter_backends.7 filter_syntax.5

EXTRA_DIST = HISTORY HONESTY README TODO

AM_CFLAGS = -g -Wall -Werror
AM_LDFLAGS = -g
