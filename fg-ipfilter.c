/*
 * Filter generator, ipfilter driver
 *
 * $Id: fg-ipfilter.c,v 1.1 2001/10/04 14:02:43 matthew Exp $
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

static int cb_ipfilter(const struct filterent *ent, void *misc)
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

	/* XXX - all our rules have to be "quick".  ipfilter
	 * people seem to prefer having defaults at the top.
	 */
	APPS(rule, "quick");

	/* access list name */
	if(ent->iface) APPSS2(rule, "on", ent->iface);

	/* protocol */
	switch(ent->proto) {
	case 0: break;
	case TCP: APPSS2(rule, "proto", "tcp"); break;
	case UDP: APPSS2(rule, "proto", "udp"); break;
	default: abort();
	}

	APPS(rule, "from");
	rule = appip(rule, ent->srcaddr);
	rule = appport(rule, ent->u.ports.src, NEG(SPORT));
	APPS(rule, "to");
	rule = appip(rule, ent->dstaddr);
	rule = appport(rule, ent->u.ports.dst, NEG(DPORT));

	if(ent->proto && (ent->target == F_ACCEPT))
		APPS(rule, "keep state");

	puts(rule);
	free(rule);
	return 1;
}

int fg_ipfilter(struct filter *filter)
{
	filter_unroll(&filter);
	return filtergen_cprod(filter, cb_ipfilter, NULL);
}
