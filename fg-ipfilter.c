/*
 * Filter generator, ipfilter driver
 *
 * $Id: fg-ipfilter.c,v 1.6 2002/04/14 11:38:40 matthew Exp $
 */

/* TODO:
 * - does this need skeleton routines?
 * - implement grouping
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

static char *appip(char *r, const char *h)
{
	if(!h) return APPS(r, "any");
	return APPS(r, h);
}

static char *appport(char *r, const char *p, int neg)
{
	char *f;
	if(!p) return r;

	APPS(r, "port");
	if(!(f = strchr(p, ':')))
		return APPSS2(r, neg ? "!=" : "=", p);

	/* XXX - const? hardly :-) */
	*f = 0;

	APPS(r, p);
	APPSS2(r, neg ? "><" : "<>", f+1);
	*f = ':';
	return r;
}

static char *appicmp(char *r, const char *h, int neg)
{
	if(!h) return r;
	if(neg) APPS(r, "!");
	return APPSS2(r, "icmp-type", h);
}

static int cb_ipfilter_rule(const struct filterent *ent, void *misc)
{
	char *rule = NULL;

	/* target first */
	switch(ent->target) {
	case F_ACCEPT:	APP(rule, "pass"); break;
	case F_DROP:	APP(rule, "block"); break;
	case F_REJECT:
		/* XXX - can this violate the rule about icmp errors
		 * about icmp errors? */
		if (ent->proto == TCP)
			APP(rule, "block return-rst");
		else
			APP(rule, "block return-icmp-as-dest(port-unr)");
		break;
	default: abort();
	}

	/* in or out? */
	switch(ent->direction) {
	case F_INPUT:	APPS(rule, "in"); break;
	case F_OUTPUT:	APPS(rule, "out"); break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	if(ent->log) APPS(rule, "log");

	/* XXX - All our rules have to be "quick", but ipfilter
	 * people seem to prefer having defaults at the top.
	 * It would be nice to improve the output readability.
	 */
	APPS(rule, "quick");

	/* access list name */
	if(ent->iface) APPSS2(rule, "on", ent->iface);

	/* protocol */
	switch(ent->proto) {
	case 0: break;
	case TCP: APPSS2(rule, "proto", "tcp"); break;
	case UDP: APPSS2(rule, "proto", "udp"); break;
	case ICMP: APPSS2(rule, "proto", "icmp"); break;
	default: abort();
	}

	APPS(rule, "from");
	rule = appip(rule, ent->srcaddr);
	rule = appport(rule, ent->u.ports.src, NEG(SPORT));
	APPS(rule, "to");
	rule = appip(rule, ent->dstaddr);
	rule = appport(rule, ent->u.ports.dst, NEG(DPORT));

	rule = appicmp(rule, ent->u.icmp, NEG(ICMPTYPE));

	if(ent->proto && (ent->target == F_ACCEPT))
		APPS(rule, "keep state");

	puts(rule);
	free(rule);
	return 1;
}

int fg_ipfilter(struct filter *filter, int flags)
{
	fg_callback cb_ipfilter = {
	rule:	cb_ipfilter_rule,
	};

	filter_nogroup(filter);
	filter_unroll(&filter);
	return filtergen_cprod(filter, &cb_ipfilter, NULL);
}
