/*
 * Filter generator, Cisco IOS driver
 *
 * $Id: fg-cisco.c,v 1.3 2001/10/04 14:02:43 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

static char *appip(char *r, const char *h)
{
	if(!h) return APPS(r, "any");

	/* XXX - split netmask, don't expect host */
	return APPS2(r, "host ", h);
}

static char *appport(char *r, const char *p, int neg)
{
	char *f;

	if(!p) return APPS(r, "any");

	if(!(f = strchr(p, ':')))
		return APPS2(r, neg ? "ne " : "eq ", p);

	/* XXX - const? hardly :-) */
	*f = 0;

	if(neg) abort();
	APPS(r, "range");
	APPSS2(r, p, f+1);
	*f = ':';
	return r;
}

static int cb_cisco(const struct filterent *ent, void *misc)
{
	char *rule = NULL, *rule_r = NULL;
	int needret = 0, needports = 1;

	APP(rule, "access-list ");
	APP(rule_r, "access-list ");

	/* access list name */
	if(ent->iface) {
		APP2(rule, ent->iface, "-");
		APP2(rule_r, ent->iface, "-");
	}
	switch(ent->direction) {
	case F_INPUT:	APP(rule, "IN"); APP(rule_r, "OUT"); break;
	case F_OUTPUT:	APP(rule, "OUT"); APP(rule_r, "IN"); break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	/* target */
	switch(ent->target) {
	case F_ACCEPT: APPS(rule, "permit"); APPS(rule_r, "permit"); break;
	case F_DROP: APPS(rule, "deny"); APPS(rule_r, "deny"); break;
	case F_REJECT: fprintf(stderr, "Cisco IOS does not support REJECT\n"); return -1;
	default: abort();
	}

	/* protocol */
	switch(ent->proto) {
	case 0:
		APPS(rule, "ip"); APPS(rule_r, "ip");
		needports = 0;
		break;
	case TCP:
		needret++;
		APPS(rule, "tcp"); APPS(rule_r, "tcp");
		break;
	case UDP:
		needret++;
		APPS(rule, "udp"); APPS(rule_r, "udp");
		break;
	default: abort();
	}

	rule = appip(rule, ent->srcaddr);
	rule = appip(rule, ent->dstaddr);
	rule_r = appip(rule_r, ent->dstaddr);
	rule_r = appip(rule_r, ent->srcaddr);

	if(needports) {
		rule = appport(rule, ent->u.ports.src, NEG(SPORT));
		rule = appport(rule, ent->u.ports.dst, NEG(DPORT));
		rule_r = appport(rule_r, ent->u.ports.dst, NEG(DPORT));
		rule_r = appport(rule_r, ent->u.ports.src, NEG(SPORT));
	}

	if(ent->proto == TCP) {
		APPS(rule_r, "established");
	}

	puts(rule);
	if(needret) puts(rule_r);

	free(rule); free(rule_r);
	return 1 + !!needret;
}

int fg_cisco(struct filter *filter)
{
	printf("# Warning: This backend is not complete and "
		"can generate broken rulesets.\n");
	filter_unroll(&filter);
	return filtergen_cprod(filter, cb_cisco, NULL);
}
