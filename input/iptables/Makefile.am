bin_PROGRAMS = scan parse emit

scan_SOURCES = parser.y scanner.l scan.c

parse_SOURCES = parser.y scanner.l parse.c

emit_SOURCES = parser.y scanner.l emit.c

AM_CFLAGS = -D_GNU_SOURCE
AM_LDFLAGS = -g
AM_YFLAGS = -d
