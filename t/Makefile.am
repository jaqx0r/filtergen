TESTS = t_01_ccomment \
	t_02_hashcomment \
	t_03_hashnonewline \
	t_04_keywords \
	t_05_punct \
	t_06_numbers \
	t_07_names \
	t_08_networknames \
	t_09_badcomment \
	t_10_goodcomment \
	t_11_includeone \
	t_12_includeloop \
	t_13_includetwo \
	t_14_includeinline \
	t_15_includenotfound \
	t_16_emptyfilter \
	t_17_oneemptyrule \
	t_18_manyemptyrules \
	t_19_simplerule \
	t_20_fullrule \
	t_21_compoundarg \
	t_22_hostnames \
	t_23_ports \
	t_24_log \
	t_25_target \
	t_26_negation \
	t_27_compoundspecifier \
	t_28_routing \
	t_29_nocommentend \
	t_30_chaingroup \
	t_31_chaingroupbad \
	t_32_syntaxexamples \
	t_33_nestedparse \
	t_34_ast \
	t_35_rule_list

check_PROGRAMS = scan parse emit

scan_SOURCES = scan.c
scan_LDADD = ../scanner.o

parse_SOURCES = parse.c
parse_LDADD = ../scanner.o ../parser.o

emit_SOURCES = emit.c
emit_LDADD = ../scanner.o ../parser.o

EXTRA_DIST = $(top_srcdir)/t/data/* testlib $(TESTS)

AM_CFLAGS = -g -Wall -Werror