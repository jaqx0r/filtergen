/*
 * filter compilation routines
 *
 * $Id: gen.c,v 1.3 2001/10/06 18:25:16 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		fg_callback cb, void *misc);

int __fg_applylist(struct filterent *e, const struct filter *f,
			fg_callback cb, void *misc)
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
		fg_callback cb, void *misc);

int __fg_applyone(struct filterent *e, const struct filter *f,
		fg_callback cb, void *misc)
{
#define NA(t)							\
	if(e->t) {						\
		fprintf(stderr, "filter has already defined a " #t "\n"); \
		return -1;					\
	}

	switch(f->type) {
	case F_TARGET:
		NA(target);
		if(f->child) abort();
#if 0		/* XXX - surely this never made sense? */
		if(f->next) abort();
#endif
		e->target = f->u.target;
		break;

	case F_INPUT: case F_OUTPUT:
		NA(direction);
		NA(iface);
		e->direction = f->type;
		e->iface = f->u.iface;
		break;

	case F_LOG: e->log = f->u.log; break;

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
		fg_callback cb, void *misc)
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


int filtergen_cprod(struct filter *filter, fg_callback cb, void *misc)
{
	struct filterent e;
	memset(&e, 0, sizeof(e));
	return __fg_applylist(&e, filter, cb, misc);
}
