/*
 * Filter generator, ipfilter driver
 *
 * $Id: fg-ipfilter.c,v 1.12 2003/04/02 11:07:32 matthew Exp $
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

static char *appip(char *r, const struct addr_spec *h)
{
	if(!h->addrstr) return APPS(r, "any");
	APPS(r, h->addrstr);
	if(h->maskstr) APP2(r, "/", h->maskstr);
	return r;
}

static char *appport(char *r, const struct port_spec *p, int neg)
{
	if(!p->minstr) return r;

	APPS(r, "port");
	if(!p->maxstr)
		return APPSS2(r, neg ? "!=" : "=", p->minstr);

	APPS(r, p->minstr);
	APPSS2(r, neg ? "><" : "<>", p->maxstr);
	return r;
}

static char *appicmp(char *r, const char *h, int neg)
{
	if(!h) return r;
	if(neg) APPS(r, "!");
	return APPSS2(r, "icmp-type", h);
}

static int cb_ipfilter_rule(const struct filterent *ent, struct fg_misc *misc)
{
	char *rule = NULL;

	/* target first */
	switch(ent->target) {
	case T_ACCEPT:	APP(rule, "pass"); break;
	case DROP:	APP(rule, "block"); break;
	case T_REJECT:
		/* XXX - can this violate the rule about icmp errors
		 * about icmp errors? */
		if (ent->proto.num == IPPROTO_TCP)
			APP(rule, "block return-rst");
		else
			APP(rule, "block return-icmp-as-dest(port-unr)");
		break;
	default: abort();
	}

	/* in or out? */
	switch(ent->direction) {
	case INPUT:	APPS(rule, "in"); break;
	case OUTPUT:	APPS(rule, "out"); break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	if(ESET(ent,LOG)) APPS(rule, "log");

	/* XXX - All our rules have to be "quick", but ipfilter
	 * people seem to prefer having defaults at the top.
	 * It would be nice to improve the output readability.
	 */
	APPS(rule, "quick");

	/* access list name */
	if(ent->iface) APPSS2(rule, "on", ent->iface);

	/* protocol */
	if(ent->proto.name)
		APPSS2(rule, "proto", ent->proto.name);

	APPS(rule, "from");
	rule = appip(rule, &ent->srcaddr);
	rule = appport(rule, &ent->u.ports.src, NEG(SPORT));
	APPS(rule, "to");
	rule = appip(rule, &ent->dstaddr);
	rule = appport(rule, &ent->u.ports.dst, NEG(DPORT));

	rule = appicmp(rule, ent->u.icmp, NEG(ICMPTYPE));

	if(ent->proto.name && (ent->target == T_ACCEPT) && !ent->oneway)
		APPS(rule, "keep state");

	oputs(rule);
	free(rule);
	return 1;
}

int fg_ipfilter(struct filter *filter, int flags)
{
	struct fg_misc misc = { flags, NULL };
	fg_callback cb_ipfilter = {
	rule:	cb_ipfilter_rule,
	};

	filter_nogroup(filter);
	filter_unroll(&filter);
	filter_apply_flags(filter, flags);
	return filtergen_cprod(filter, &cb_ipfilter, &misc);
}
