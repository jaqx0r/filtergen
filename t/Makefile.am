TESTS = t_01_ccomment \
	t_02_hashcomment \
	t_03_nothing \
	t_04_keywords \
	t_05_punct \
	t_06_numbers \
	t_07_names \
	t_08_networknames \
	t_09_badcomment \
	t_10_goodcomment

check_PROGRAMS = scan

scan_SOURCES = scan.c
scan_LDADD = ../scanner.o

EXTRA_DIST = data/*
