/* $Id: fg-util.c,v 1.1 2001/10/03 19:01:54 matthew Exp $ */

#include <stdlib.h>
#include <string.h>

char *strapp(char *s, const char *n)
{
	size_t l;
	if(!n) return s;
	if(!s) return strdup(n);
	l = strlen(s) + strlen(n) + 1;
	s = realloc(s, l);
	return strcat(s, n);
}
