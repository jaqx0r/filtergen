TESTS = comment \
	tokens

check_PROGRAMS = scan parse emit

scan_SOURCES = scan.c
scan_LDADD = ../in_iptables.a

parse_SOURCES = parse.c
parse_LDADD = ../in_iptables.a

emit_SOURCES = emit.c
emit_LDADD = ../in_iptables.a

AM_CFLAGS = -g -Wall -Werror -I$(top_srcdir)/input/iptables