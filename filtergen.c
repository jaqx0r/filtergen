/*
 * filter compilation front-end
 *
 * $Id: filtergen.c,v 1.4 2001/10/03 19:51:48 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "filter.h"


struct filtyp {
	char *name;
	filtergen *compiler;
} filter_types[] = {
	{ "iptables", fg_iptables, },
	{ "ipchains", fg_ipchains, },
	{ "cisco", fg_cisco, },
	{ NULL, },
};


int main(int argc, char **argv)
{
	extern FILE *yyin;
	struct filter *f;
	int l;
	time_t t;
	char buf[100];
	char *ftn;
	struct filtyp *ft;

	if((argc > 1) && strcmp(argv[1], "-")) {
		if(!(yyin = fopen(argv[1], "r"))) {
			fprintf(stderr, "%s: can't open file \"%s\"\n",
					argv[0], argv[1]);
			return 1;
		}
	}

	ftn = (argc > 2) ? argv[2] : "iptables";
	for(ft = filter_types; ft->name; ft++)
		if(!strcmp(ftn, ft->name))
			break;
	if(!ft) {
		fprintf(stderr, "%s: unknown filter type \"%s\"\n", *argv, ftn);
		return 1;
	}

	f = filter_parse_list();
	if (!f) {
		fprintf(stderr, "couldn't parse file\n");
		return 1;
	}

	strftime(buf, sizeof(buf)-1, "%a %b %e %H:%M:%S %Z %Y",
			localtime((time(&t),&t)));
	fprintf(stderr, "# filter generated from %s at %s\n",
			argv[1] ?: "standard input",
			buf);
	l = ft->compiler(f);

	if(l < 0) {
		fprintf(stderr, "an error occurred: most likely the filters defined make no sense\n");
		return 1;
	}
	fprintf(stderr, "generated %d rules\n", l);
	return 0;
}
