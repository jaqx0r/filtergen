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
	t_19_simplerule

check_PROGRAMS = scan parse

scan_SOURCES = scan.c
scan_LDADD = ../scanner.o

parse_SOURCES = parse.c
parse_LDADD = ../scanner.o ../parser.o

EXTRA_DIST = data/* testlib $(TESTS)
