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
	t_35_rule_list \
	t_36_rule \
	t_37_specifier_list \
	t_38_specifier \
	t_39_negated_specifier \
	t_40_direction \
	t_41_target \
	t_42_host \
	t_43_port \
	t_44_protocol \
	t_45_icmptype \
	t_46_routing \
	t_47_subrule_list \
	t_48_chaingroup \
	t_49_convertspeclist \
	t_50_convertrulelist \
	t_51_convertdirection \
	t_52_converthost \
	t_53_convertproto \
	t_54_convertport \
	t_55_converticmptype \
	t_56_convertrouting \
	t_57_convertchains \
	t_58_convertcompound \
	t_59_convertnested \
	t_60_bigugly \
	t_61_convertlog \
	t_62_convertoptions \
	t_63_includedir \
	t_64_includedirwfile \
	t_65_includedirwdir \
	t_66_includedirtwofiles \
	t_67_includeemptyfile \
	t_68_noop \
	t_69_includeemptynoop \
	t_70_includesemicolon

check_PROGRAMS = scan parse emit convert factorise

scan_SOURCES = scan.c
scan_LDADD = ../scanner.o ../parser.o

parse_SOURCES = parse.c
parse_LDADD = ../scanner.o ../parser.o

emit_SOURCES = emit.c
emit_LDADD = ../scanner.o ../parser.o

convert_SOURCES = convert.c
convert_LDADD = ../scanner.o ../parser.o ../glue.o ../filter.o ../fg-util.o

factorise_SOURCES = factorise.c
factorise_LDADD = ../scanner.o ../parser.o

EXTRA_DIST = $(top_srcdir)/t/data/*.in $(top_srcdir)/t/data/*.out \
	testlib $(TESTS)

AM_CFLAGS = -g -Wall -Werror -I$(top_srcdir)
