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
#include "resolver.h"

int yyparse(void *);
void yyrestart(FILE *);
extern struct filter * convert(struct ast_s * n);

static FILE *outfile;

void usage(char * prog) {
    fprintf(stderr, "Usage: %s [-chV] [-t backend] [-o output] input\n", prog);
    fprintf(stderr, "       %s [-chV] [-t backend] [-o output] -F policy\n\n", prog);
    fprintf(stderr, "Options:\n");

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --compile/-c              compile only, no generate\n");
#else
    fprintf(stderr, "           -c              compile only, no generate\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --target/-t target        generate for target (default: iptables)\n");
#else
    fprintf(stderr, "          -t target        generate for target (default: iptables)\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --flush/-F policy         don't process input, generate flush rules\n");
#else
    fprintf(stderr, "         -F policy         don't process input, generate flush rules\n");
#endif

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

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --version/-V              show program version\n");
#else
    fprintf(stderr, "           -V              show program version\n");
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
    const char * name;
    filtergen * compiler;
    filter_flush * flusher;
} filter_types[] = {
    { "iptables", fg_iptables, flush_iptables, },
    { "ipchains", fg_ipchains, flush_ipchains, },
    { "ipfilter", fg_ipfilter, NULL },
    { "cisco", fg_cisco, NULL },
    { NULL, NULL, NULL },
};

#ifdef HAVE_GETOPT_H
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"compile", no_argument, 0, 'c'},
    {"target", required_argument, 0, 't'},
    {"output", required_argument, 0, 'o'},
    {"flush", required_argument, 0, 'F'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0}
};
# define GETOPT(x, y, z) getopt_long(x, y, z, long_options, NULL)
#else
# define GETOPT(x, y, z) getopt(x, y, z)
#endif

int main(int argc, char **argv) {
    struct filter *f;
    int l;
    time_t t;
    char buf[100];
    char *filename = NULL, *ftn = NULL, *ofn = NULL;
    struct filtyp *ft = NULL; 
    int flags = 0;
    char *progname;
    int arg;
    enum filtertype flushpol = T_ACCEPT;

    progname = argv[0];

    while ((arg = GETOPT(argc, argv, "hco:t:F:V")) > 0) {
	switch (arg) {
	  case ':':
	    usage(progname);
	    exit(1);
	    break;
	  case 'h':
	    usage(progname);
	    exit(0);
	    break;
	  case 'c':
	    flags |= FF_NOSKEL;
	    break;
	  case 'o':
	    ofn = strdup(optarg);
	    break;
	  case 't':
	    ftn = strdup(optarg);
	    break;
	  case 'F':
	    flags |= FF_FLUSH;
	    if (!strcasecmp(optarg, "accept")) {
		flushpol = T_ACCEPT;
	    } else if (!strcasecmp(optarg, "drop")) {
		flushpol = DROP;
	    } else if (!strcasecmp(optarg, "reject")) {
		flushpol = T_REJECT;
	    } else {
		fprintf(stderr, "%s: flush policy unrecofgnised: %s\n", progname, optarg);
		usage(progname);
		exit(1);
	    }
	    break;
	  case 'V':
	    printf(PACKAGE " " VERSION "\n");
	    exit(0);
	    break;
	  default:
	    break;
	}
    }
    if (!(flags & FF_FLUSH)) {
	if (optind >= argc) {
	    usage(progname);
	    exit(1);
	} else
	    filename = argv[optind++];
    }

    if (ofn) {
	/* XXX - open a different tempfile, and rename on success */
	outfile = fopen(ofn, "w");
	if(!outfile) {
	    fprintf(stderr, "%s: can't open output file \"%s\"\n", progname, ofn);
	    return 1;
	}
    } else
	outfile = stdout;

    if(!ftn || !*ftn) ftn = strdup("iptables");
    for (ft = filter_types; ft->name; ft++)
	if (!strcmp(ftn, ft->name))
	    break;
    if (!ft->name) {
	fprintf(stderr, "%s: target filter unrecognised: %s\n", progname, ftn);
	usage(progname);
	exit(1);
    }

    /* What to do, then? */
    if(flags & FF_FLUSH) {
	/* Just flush it */
	l = ft->flusher(flushpol);
    } else {
	/* Compile from a file */
	if(filename && !strcmp(filename, "-")) filename = NULL;

	if(filter_fopen(filename)) return 1;

	{
	    struct ast_s ast;

	    if (yyparse((void *) &ast) == 0) {
		resolve(&ast);
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
		filename ?: "standard input", ft->name, buf);
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
