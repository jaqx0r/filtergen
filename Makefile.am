SUBDIRS = . t

bin_PROGRAMS = filtergen

filtergen_SOURCES = \
	tmpmain.c \
	scanner.l \
	parser.y

#	filtergen.c \
#	gen.c \
#	filter.c \
#	filterlex.l \
#	fg-util.c \
#	fg-iptables.c \
#	fg-ipchains.c \
#	fg-ipfilter.c \
#	fg-cisco.c

headers = filter.h util.h tokens.h

mans = filtergen.8 filter_backends.7 filter_syntax.5

man_MANS = $(mans)

EXTRA_DIST = HISTORY HONESTY README TODO $(headers) $(mans)

AM_CFLAGS = -g -Wall -Werror
AM_LDFLAGS = -g
AM_YFLAGS = -d