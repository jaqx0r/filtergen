/* $Id: fg-util.c,v 1.2 2002/08/20 22:54:38 matthew Exp $ */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *strapp(char *s, const char *n)
{
	size_t l;
	if(!n) return s;
	if(!s) return strdup(n);
	l = strlen(s) + strlen(n) + 1;
	s = realloc(s, l);
	return strcat(s, n);
}

int str_to_int(const char *s, int *i)
{
	long m;
	char *e;

	m = strtol(s, &e, 10);
	if(*e) return -1;
	*i = m;
	return 0;
}

char *int_to_str_dup(int i)
{
	char buf[100];
	snprintf(buf, sizeof(buf)-1, "%d", i);
	buf[sizeof(buf)-1] = 0;
	return strdup(buf);
}
