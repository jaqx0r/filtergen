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
#include "input/input.h"

static FILE *outfile;

/** Print the program usage to stderr. */
void usage(char * prog) {
    fprintf(stderr, "Usage: %s [-chV] [-t backend] [-o output] input\n", prog);
    fprintf(stderr, "       %s [-chV] [-t backend] [-o output] -F policy\n\n", prog);
    fprintf(stderr, "Options:\n");

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --compile/-c              compile only, no generate\n");
#else
    fprintf(stderr, "\t-c\t\tcompile only, no generate\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --no-resolve/-r           don't resolve names\n");
#else
    fprintf(stderr, "\t-r\t\tdon't resolve names\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --source/-s source        source language (default: filtergen)\n");
#else
    fprintf(stderr, "\t-s source\tsource language (default: filtergen)\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --target/-t target        generate for target (default: iptables)\n");
#else
    fprintf(stderr, "\t-t target\tgenerate for target (default: iptables)\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --flush/-F policy         don't process input, generate flush rules\n");
#else
    fprintf(stderr, "\t-F policy\tdon't process input, generate flush rules\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --output/-o filename      write the generated packet filter to filename\n");
#else
    fprintf(stderr, "\t-o filename\twrite the generated packet filter to filename\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --help/-h                 show this help\n");
#else
    fprintf(stderr, "\t-h\t\tshow this help\n");
#endif

#ifdef HAVE_GETOPT_H
    fprintf(stderr, " --version/-V              show program version\n");
#else
    fprintf(stderr, "\t-V\t\tshow program version\n");
#endif
}

/** Output a string, and appends a newline, to the target file. */
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

/** Write a string, printf style, to the target file. */
int oprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return vfprintf(outfile, fmt, args);
}

/** Function pointer table containing source parsers */
struct source_parser_s {
    const char * name;
    source_parser * parser;
} source_parsers[] = {
    { "filtergen", filtergen_source_parser },
    { "iptables-save", ipts_source_parser },
    { NULL, NULL }
};

/** Function pointer table containing target filter types and the emitter
 * functions */
struct filtyp {
    const char * name;
    filtergen * compiler;
    filter_flush * flusher;
} filter_types[] = {
    { "iptables", fg_iptables, flush_iptables, },
    { "iptablesrestore", fg_iptablesrestore, flush_iptablesrestore, },
    { "ipchains", fg_ipchains, flush_ipchains, },
    { "ipfilter", fg_ipfilter, NULL },
    { "cisco", fg_cisco, NULL },
    { "filtergen", emit_filtergen, NULL },
    { NULL, NULL, NULL }
};

#ifdef HAVE_GETOPT_H
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"compile", no_argument, 0, 'c'},
    {"target", required_argument, 0, 't'},
    {"output", required_argument, 0, 'o'},
    {"flush", required_argument, 0, 'F'},
    {"version", no_argument, 0, 'V'},
    {"no-resolve", no_argument, 0, 'r'},
    {"source", required_argument, 0, 's'},
    {0, 0, 0, 0}
};
# define GETOPT(x, y, z) getopt_long(x, y, z, long_options, NULL)
#else
# define GETOPT(x, y, z) getopt(x, y, z)
#endif

/** Program entry point. */
int main(int argc, char **argv) {
    struct filter *f;
    int l;
    char *filename = NULL, *ftn = NULL, *ofn = NULL, * source_name = NULL;
    struct source_parser_s * sp;
    struct filtyp *ft = NULL;
    int flags = 0;
    char *progname;
    int arg;
    enum filtertype flushpol = T_ACCEPT;
    int resolve_names = 1;

    progname = argv[0];

    while ((arg = GETOPT(argc, argv, "hco:t:F:Vrs:")) > 0) {
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
        case 's':
            source_name = strdup(optarg);
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
            printf("filtergen " VERSION "\n");
            exit(0);
            break;
        case 'r':
            resolve_names = 0;
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

    /* work out which source parser to use */
    if (!source_name || !*source_name) source_name = strdup("filtergen");
    for (sp = source_parsers; sp->name; ++sp) {
        if (!strcmp(source_name, sp->name))
            break;
    }
    if (!sp->name) {
        fprintf(stderr, "%s: source parser unrecognised: %s", progname, source_name);
        usage(progname);
        exit(1);
    }

    /* work out which target emitter to use */
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
        FILE * file;

        /* Compile from a file */
        if(filename && !strcmp(filename, "-")) filename = NULL;

        if (filename) {
            /** FIXME: make more effort to find the file */
            if (!(file = fopen(filename, "r"))) {
                fprintf(stderr, "can't open file \"%s\"", filename);
            }
        } else {
            file = stdin;
        }
        f = sp->parser(file, resolve_names);
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
