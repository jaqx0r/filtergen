/*
 * filter compilation front-end
 *
 * $Id: filtergen.c,v 1.17 2002/11/11 20:51:38 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "filter.h"


static FILE *outfile;

int oputs(const char *s)
{
	int r = 0;
	if(s) {
		r = fputs(s, outfile);
		if(r > 0) {
			fputc('\n', outfile);
			r++;
		}
	}
	return r;
}

int oprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	return vfprintf(outfile, fmt, args);
}


struct filtyp {
	char *name;
	filtergen *compiler;
	filter_flush *flusher;
} filter_types[] = {
	{ "iptables", fg_iptables, flush_iptables, },
	{ "ipchains", fg_ipchains, flush_ipchains, },
	{ "ipfilter", fg_ipfilter, },
	{ "cisco", fg_cisco, },
	{ NULL, },
};


int main(int argc, char **argv)
{
	struct filter *f;
	int l;
	time_t t;
	char buf[100];
	char *filepol = NULL, *ftn = NULL, *ofn = NULL;
	struct filtyp *ft;
	int flags = 0;
	char *progname;
	int arg;

	progname = argv[0];

	while((arg = getopt(argc, argv, "nlmrho:t:F")) > 0) {
		switch(arg) {
		case 'n': flags |= FF_NOSKEL; break;
		case 'l': flags |= FF_LSTATE; break;
		case 'm': flags |= FF_LOOKUP; break;
		case 'h': flags |= FF_LOCAL; break;
		case 'r': flags |= FF_ROUTE; break;
		case 'o': ofn = strdup(optarg); break;
		case 't': ftn = strdup(optarg); break;
		case 'F': flags |= FF_FLUSH; break;
		default: return 1;
		}
	}

	if((flags & FF_LOCAL) && (flags & FF_ROUTE)) {
		fprintf(stderr, "the -h and -r options are mutually exclusive\n");
		return 1;
	}

	if (ofn) {
		/* XXX - open a different tempfile, and rename on success */
		outfile = fopen(ofn, "w");
		if(!outfile) {
			fprintf(stderr, "can't open output file \"%s\"\n", ofn);
			return 1;
		}
	} else
		outfile = stdout;

	if(!ftn || !*ftn) ftn = "iptables";
	for(ft = filter_types; ft->name; ft++)
		if(!strcmp(ftn, ft->name))
			break;
	if(!ft->name) {
		fprintf(stderr, "%s: unknown filter type \"%s\"\n", progname, ftn);
		return 1;
	}

	/* Extra arg (either filename or flush policy) */
	if((filepol = argv[optind])) {
		/* Extra args? */
		while(argv[++optind])
			fprintf(stderr, "%s: extra argument \"%s\" ignored\n",
					progname, argv[optind]);
	}

	/* What to do, then? */
	if(flags & FF_FLUSH) {
		/* Just flush it */
		enum filtertype flushpol;

		if(!filepol || !strcasecmp(filepol, "accept"))
			flushpol = T_ACCEPT;
		else if(!strcasecmp(filepol, "drop"))
			flushpol = DROP;
		else if(!strcasecmp(filepol, "reject"))
			flushpol = T_REJECT;
		else {
			fprintf(stderr, "invalid flush policy \"%s\"\n", filepol);
			return 1;
		}

		l = ft->flusher(flushpol, flags);

	} else {
		/* Compile from a file */
		if(filepol && !strcmp(filepol, "-")) filepol = NULL;

		if(filter_fopen(filepol)) return 1;

		f = filter_parse_list();
		if (!f) {
			fprintf(stderr, "couldn't parse file\n");
			return 1;
		}

		strftime(buf, sizeof(buf)-1, "%a %b %e %H:%M:%S %Z %Y",
				localtime((time(&t),&t)));
		oprintf("# filter generated from %s via %s backend at %s\n",
			filepol ?: "standard input", ft->name, buf);
		l = ft->compiler(f, flags);
	}

	if(ofn) fclose(outfile);

	if(l < 0) {
		fprintf(stderr, "an error occurred: most likely the filters defined make no sense\n");
		if(ofn) unlink(ofn);
		return 1;
	}
	fprintf(stderr, "generated %d rules\n", l);
	return 0;
}
