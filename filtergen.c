/*
 * filter compilation front-end
 *
 * $Id: filtergen.c,v 1.11 2002/04/14 15:18:47 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "filter.h"


struct filtyp {
	char *name;
	filtergen *compiler;
} filter_types[] = {
	{ "iptables", fg_iptables, },
	{ "ipchains", fg_ipchains, },
	{ "ipfilter", fg_ipfilter, },
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
	char *fn = NULL, *ftn = NULL;
	struct filtyp *ft;
	int flags = 0;
	char *progname;
	int arg;

	progname = argv[0];

	while((arg = getopt(argc, argv, "nl")) > 0) {
		switch(arg) {
		case 'n': flags |= FF_NOSKEL; break;
		case 'l': flags |= FF_LSTATE; break;
		case '?': return 1;
		}
	}

	fn = argv[optind];
	if(fn && strcmp(fn, "-")) {
		if(!(yyin = fopen(fn, "r"))) {
			fprintf(stderr, "%s: can't open file \"%s\"\n",
					progname, fn);
			return 1;
		}
	} else {
		fn = NULL;
	}

	if(argc > optind) ftn = argv[optind+1];
	if(!ftn || !*ftn) ftn = "iptables";
	for(ft = filter_types; ft->name; ft++)
		if(!strcmp(ftn, ft->name))
			break;
	if(!ft->name) {
		fprintf(stderr, "%s: unknown filter type \"%s\"\n", progname, ftn);
		return 1;
	}

	strftime(buf, sizeof(buf)-1, "%a %b %e %H:%M:%S %Z %Y",
			localtime((time(&t),&t)));
	printf("# filter generated from %s via %s backend at %s\n",
		fn ?: "standard input", ft->name, buf);

	f = filter_parse_list();
	if (!f) {
		fprintf(stderr, "couldn't parse file\n");
		return 1;
	}
	l = ft->compiler(f, flags);

	if(l < 0) {
		fprintf(stderr, "an error occurred: most likely the filters defined make no sense\n");
		return 1;
	}
	fprintf(stderr, "generated %d rules\n", l);
	return 0;
}
