SUBDIRS = . t

sbin_PROGRAMS = filtergen
sbin_SCRIPTS = fgadm

filtergen_SOURCES = \
	filtergen.c \
	gen.c \
	filter.c \
	fg-util.c \
	fg-iptables.c \
	fg-iptrestore.c \
	fg-ipchains.c \
	fg-ipfilter.c \
	fg-cisco.c \
	parser.y \
	scanner.l \
	glue.c \
	resolver.c \
	icmpent.c

filtergen_LDADD = @GETOPT_LIBS@ @LIBPROFILER@ @GCOV_LIBS@

headers = filter.h util.h ast.h resolver.h icmpent.h

mans = filter_backends.7 filter_syntax.5 filtergen.8 fgadm.8

man_MANS = $(mans)

dist_sysconf_DATA = fgadm.conf rules.filter

dist_pkgex_DATA = $(top_srcdir)/examples/*.filter

dist_pkgdoc_DATA = $(top_srcdir)/doc/*

EXTRA_DIST = HISTORY HONESTY README TODO $(headers) $(mans) filtergen.spec extras/*

AM_CFLAGS = -D_GNU_SOURCE @GCOV_CFLAGS@
AM_LDFLAGS = -g
AM_YFLAGS = -d
