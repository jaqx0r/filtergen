/*
 * filter compilation routines
 *
 * $Id: gen.c,v 1.11 2002/07/21 12:34:44 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"

void applydefaults(struct filterent *e, long flags)
{
	if(!e->rtype) {
		if(flags & FF_LOCAL) e->rtype = LOCALONLY;
		else if(flags & FF_ROUTE) e->rtype = ROUTEDONLY;
	}
}

int checkmatch(const struct filterent *e)
{
	int r = 0;
#define	MUST(t)							\
	do if(!(e->t)) {					\
		fprintf(stderr, #t " missing from filter\n");	\
		r++;						\
	} while(0)
	if(!e->subgroup)
		MUST(target);
	if(!e->groupname) {
		MUST(direction);
		MUST(iface);
	}
#undef MUST

	if((e->u.ports.src || e->u.ports.dst)
	&& (e->proto != TCP) && (e->proto != UDP)) {
		fprintf(stderr, "can only use ports with tcp or udp\n");
		r++;
	}

	if(e->u.icmp && (e->proto != ICMP)) {
		fprintf(stderr, "icmptype can only be used with icmp\n");
		r++;
	}

	if((e->rtype == LOCALONLY) && (e->target == F_MASQ)) {
		fprintf(stderr, "\"local\" and masquerading are incompatible\n");
		r++;
	}

	return r;
}


int __fg_apply(struct filterent *e, const struct filter *f,
		fg_callback *cb, struct fg_misc *misc);

int __fg_applylist(struct filterent *e, const struct filter *f,
			fg_callback *cb, struct fg_misc *misc)
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
		fg_callback *cb, struct fg_misc *misc);

int __fg_applyone(struct filterent *e, const struct filter *f,
		fg_callback *cb, struct fg_misc *misc)
{
#define NA(t)							\
	if(e->t) {						\
		fprintf(stderr, "filter has already defined a " #t "\n"); \
		return -1;					\
	}

	switch(f->type) {
	case F_TARGET:
		NA(target);
		e->target = f->u.target;
		break;

	case F_SUBGROUP: {
		struct filterent fe;
		int r;

		NA(subgroup);
		if(e->subgroup) {
			fprintf(stderr, "cannot compose subgroups\n");
			return -1;
		}
		if(!cb->group) {
			fprintf(stderr, "backend doesn't support grouping, but hasn't removed groups\n");
			abort();
		}
		e->target = f->type;
		e->subgroup = f->u.sub.name;

		memset(&fe, 0, sizeof(fe));
		fe.groupname = f->u.sub.name;
		cb->group(fe.groupname);
		if((r = __fg_applylist(&fe, f->u.sub.list, cb, misc)) < 0)
			return r;

		break;
	}

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
	DV(ICMPTYPE, u.icmp, icmp);
	DV(RTYPE, rtype, rtype);

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
		fg_callback *cb, struct fg_misc *misc)
{
	struct filterent e = *_e;

	/* Looks like we're all done */
	if(!f) {
		applydefaults(&e, misc->flags);
		if (checkmatch(&e)) {
			fprintf(stderr, "filter definition incomplete\n");
			return -1;
		}
		return cb->rule(&e, misc);
	}

	return __fg_applyone(&e, f, cb, misc)
		?: __fg_apply(&e, f->child, cb, misc);
}


int filtergen_cprod(struct filter *filter, fg_callback *cb, struct fg_misc *misc)
{
	struct filterent e;
	memset(&e, 0, sizeof(e));
	return __fg_applylist(&e, filter, cb, misc);
}
