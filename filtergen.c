/*
 * filter compilation front-end
 *
 * $Id: filtergen.c,v 1.1 2001/09/25 17:22:39 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "filter.h"


int checkmatch(const struct filterent *e)
{
	int r = 0;
#define	MUST(t)							\
	if(!(e->t)) {						\
		fprintf(stderr, #t " missing from filter\n");	\
		r++;						\
	}
	MUST(direction);
	MUST(target);
	MUST(iface);
#undef MUST
	return r;
}


int __fg_apply(struct filterent *e, const struct filter *f,
		filtergen cb, void *misc);

int __fg_applylist(struct filterent *e, const struct filter *f,
			filtergen cb, void *misc)
{
	/* This is the interesting one.  The filters are
	 * unrolled by now, so there's only one way to
	 * follow it */
	int c = 0;
	for(; f; f = f->next) {
		int _c = __fg_apply(e, f, cb, misc);
		if (_c < 0) return _c;
		c += _c;
	}
	return c;
}

int __fg_apply(struct filterent *_e, const struct filter *f,
		filtergen cb, void *misc);

int __fg_applyone(struct filterent *e, const struct filter *f,
		filtergen cb, void *misc)
{
#define NA(t)							\
	if(e->t) {						\
		fprintf(stderr, "filter has already defined a " #t "\n"); \
		return -1;					\
	}

	switch(f->type) {
	case F_ACCEPT: case F_DROP: case F_REJECT:
		NA(target);
		e->target = f->type;
		break;

	case F_INPUT: case F_OUTPUT:
		NA(direction);
		NA(iface);
		e->direction = f->type;
		e->iface = f->u.iface;
		break;

#define	DV(n, v, p)						\
	case F_ ## n: NA(v); e->v = f->u.p; break;
	DV(SOURCE, srcaddr, addrs);
	DV(DEST, dstaddr, addrs);
	DV(SPORT, u.ports.src, ports);
	DV(DPORT, u.ports.dst, ports);
	DV(PROTO, proto, proto);

	case F_SIBLIST:
		return __fg_applylist(e, f->u.sib, cb, misc);

	default: abort();
	}
#undef NA

	if(f->negate)
		e->whats_negated |= (1 << f->type);

	return 0;
}

int __fg_apply(struct filterent *_e, const struct filter *f,
		filtergen cb, void *misc)
{
	struct filterent e = *_e;

	/* Looks like we're all done */
	if(!f) {
		if (checkmatch(&e)) {
			fprintf(stderr, "filter definition incomplete\n");
			return -1;
		}
		return cb(&e, misc);
	}

	return __fg_applyone(&e, f, cb, misc)
		?: __fg_apply(&e, f->child, cb, misc);
}


int filtergen_cprod(struct filter *filter, filtergen cb, void *misc)
{
	struct filterent e;
	memset(&e, 0, sizeof(e));

	filter_unroll(&filter);
	return __fg_applylist(&e, filter, cb, misc);
}

int main(int argc, char **argv)
{
	extern FILE *yyin;
	struct filter *f;
	int l;
	time_t t;
	char buf[100];

	if(argc > 1) yyin = fopen(argv[1], "r");
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
	l = filtergen_cprod(f, fg_iptables, NULL);

	if(l < 0) {
		fprintf(stderr, "an error occurred: most likely the filters defined make no sense\n");
		return 1;
	}
	fprintf(stderr, "generated %d rules\n", l);
	return 0;
}
