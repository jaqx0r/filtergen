/* packet filter compiler
 *
 * Copyright (c) 2002 Matthew Kirkwood
 * Copyright (c) 2003,2004 Jamie Wilkinson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "filter.h"
#include "ast.h"

int yyparse(void *);
void yyrestart(FILE *);
extern struct filter * convert(struct ast_s * n);

static FILE *outfile;

void usage(char * prog) {
    fprintf(stderr, "Usage: %s [options] [-o output] input\n\n", prog);
    fprintf(stderr, "Options:\n");

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --output/-o filename      write the generated packet filter to filename\n");
#else
    fprintf(stderr, "          -o filename      write the generated packet filter to filename\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --help/-h                 show this help\n");
#else
    fprintf(stderr, "        -h                 show this help\n");
#endif
}

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

#ifdef HAVE_GETOPT_H
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0}
};
#endif

int main(int argc, char **argv) {
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

#ifdef HAVE_GETOPT_H
    while ((arg = getopt_long(argc, argv, "ho:", long_options, NULL)) > 0) {
	switch (arg) {
	  case ':':
	    usage(progname);
	    exit(1);
	    break;
	  case 'h':
	    usage(progname);
	    exit(0);
	    break;
	  case 'o':
	    ofn = strdup(optarg);
	    break;
	  default:
	    break;
	}
    }
    if (optind >= argc) {
	usage(progname);
    } else {
	filepol = argv[optind++];
    }
#else /* !HAVE_GETOPT_H */
    while((arg = getopt(argc, argv, "ho:")) > 0) {
	switch(arg) {
	  case 'o':
	    ofn = strdup(optarg);
	    break;
	  case 'h':
	    usage(progname);
	    exit(0);
	    break;
	  default: return 1;
	}
    }
#endif

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

	{
	    struct ast_s ast;

	    if (yyparse((void *) &ast) == 0) {
		f = convert(&ast);
		if (!f) {
		    fprintf(stderr, "couldn't convert file\n");
		    return 1;
		}
	    } else {
		fprintf(stderr, "couldn't parse file\n");
		return 1;
	    }
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
