/*
 * Filter generator, ipchains driver
 *
 * XXX - maybe some of this could be shared with the iptables one?
 *
 * $Id: fg-ipchains.c,v 1.22 2002/08/20 22:54:38 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "filter.h"
#include "util.h"


static char *appip(char *r, const struct addr_spec *h)
{
	APPS(r, h->addrstr);
	if(h->maskstr) APP2(r, "/", h->maskstr);
	return r;
}
#define	APPIP(r, h)	(r = appip(r, h))
#define	APPIP2(f, r, h)	(APPS(r, f), APPIP(r, h))


static char *appport(char *r, const struct port_spec *h)
{
	APPS(r, h->minstr);
	if(h->maxstr) APP2(r, ":", h->maxstr);
	return r;
}
#define	APPPORT(r, h)		(r = appport(r, h))
#define	APPPORT2(f, r, h)	(APPS(r, f), APPPORT(r, h))


static int cb_ipchains_rule(const struct filterent *ent, struct fg_misc *misc)
{
	char *rule = NULL, *rule_r = NULL;
	char *rulechain = NULL, *revchain = NULL;
	char *ruletarget = NULL, *revtarget = NULL;
	char *forchain = NULL, *forrevchain = NULL;
	char *fortarget = NULL, *forrevtarget = NULL;
	int needret = 0;
	int isforward = (ent->rtype != LOCALONLY);
	int orules = 0;

	/* nat rule */
	if((ent->target == F_MASQ) || (ent->target == F_REDIRECT)) {
		if((ent->target == F_MASQ) && (ent->direction == F_OUTPUT)) {
			fprintf(stderr, "can't masquerade on input\n");
			return -1;
		} else if((ent->target == F_MASQ) && (ent->direction == F_OUTPUT)) {
			fprintf(stderr, "can't redirect on output\n");
			return -1;
		}
	}

	if(ent->rtype == ROUTEDONLY) {
		fprintf(stderr, "ipchains can't do forward-only rules\n");
		return -1;
	}

	switch(ent->direction) {
	case 0:		/* This is a rule in a subgroup */
			rulechain = revchain = ent->groupname; break;
	case F_INPUT:	rulechain = "input"; revchain = "output";
			forchain = "forward"; forrevchain = "forw_out"; break;
	case F_OUTPUT:	rulechain = (ent->target == F_MASQ) ? "forw_out" : "output";
			revchain = "input";
			forchain = "forw_out"; forrevchain = "forward"; break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	if(ent->iface) {
		if(NEG(INPUT)) {
			APPS(rule, "!");
			APPS(rule_r, "!");
		}
		APPSS2(rule, "-i", ent->iface);
		APPSS2(rule_r, "-i", ent->iface);
	}

	/* state and reverse rules here */
	if(ent->proto.name) {
		APPSS2(rule, "-p", ent->proto.name);
		APPSS2(rule_r, "-p", ent->proto.name);
		switch(ent->proto.num) {
		case IPPROTO_TCP:
			APPS(rule_r, "! --syn");
			/* fall through */
		case IPPROTO_UDP:
			needret++;
		}
	}

	if(ent->srcaddr.addrstr) {
		NEGA(rule, SOURCE); NEGA(rule_r, SOURCE);
		APPIP2("-s", rule, &ent->srcaddr);
		APPIP2("-d", rule_r, &ent->srcaddr);
	}
	if(ent->dstaddr.addrstr) {
		NEGA(rule, DEST); NEGA(rule_r, DEST);
		APPIP2("-d", rule, &ent->dstaddr);
		APPIP2("-s", rule_r, &ent->dstaddr);
	}

	switch(ent->proto.num) {
	case 0: break;
	case IPPROTO_UDP: case IPPROTO_TCP:
		if(ent->u.ports.src.minstr) {
			NEGA(rule, SPORT);
			NEGA(rule_r, SPORT);
			APPPORT2("--sport", rule, &ent->u.ports.src);
			APPPORT2("--dport", rule_r, &ent->u.ports.src);
		}
		if(ent->u.ports.dst.minstr) {
			NEGA(rule, DPORT);
			NEGA(rule_r, DPORT);
			APPPORT2("--dport", rule, &ent->u.ports.dst);
			APPPORT2("--sport", rule_r, &ent->u.ports.dst);
		}
		break;
	case IPPROTO_ICMP:
		if(ent->u.icmp) {
			NEGA(rule, ICMPTYPE);
			APPSS2(rule, "--icmp-type", ent->u.icmp);
		}
		break;
	default:;
	}

	if(ent->log) APPS(rule, "-l");

	APPS(rule, "-j"); APPS(rule_r, "-j");

	switch(ent->target) {
	case F_ACCEPT:	ruletarget = revtarget =
			fortarget = forrevtarget = "ACCEPT";
			switch(ent->direction) {
			case F_INPUT: fortarget = "forw_out"; break;
			case F_OUTPUT: forrevtarget = "forw_out"; break;
			default: abort();
			}
			break;
	case F_DROP:	ruletarget = fortarget = "DENY"; needret = 0; break;
	case F_REJECT:	ruletarget = fortarget = "REJECT"; needret = 0; break;
	case F_MASQ:	ruletarget = "MASQ"; revtarget = "ACCEPT"; break;
	case F_REDIRECT:ruletarget = "REDIRECT"; revtarget = "ACCEPT"; break;
	case F_SUBGROUP:ruletarget = revtarget = ent->subgroup; break;
	default: abort();
	}

	orules++, oprintf("ipchains -A %s %s %s\n", rulechain, rule+1, ruletarget);
	if(needret) orules++, oprintf("ipchains -I %s %s %s\n", revchain, rule_r+1, revtarget);
	if(isforward) {
		orules++, oprintf("ipchains -A %s %s %s\n", forchain, rule+1, fortarget);
		if(needret) orules++, oprintf("ipchains -I %s %s %s\n", forrevchain, rule_r+1, forrevtarget);
	}

	free(rule); free(rule_r);
	return orules;
}

static int cb_ipchains_group(const char *name)
{
	oprintf("ipchains -N %s\n", name);
	return 1;
}

int fg_ipchains(struct filter *filter, int flags)
{
	int r;
	struct fg_misc misc = { flags, NULL };
	fg_callback cb_ipchains = {
	rule:	cb_ipchains_rule,
	group:	cb_ipchains_group,
	};

	filter_unroll(&filter);
	filter_apply_flags(filter, flags);
	if(!(flags & FF_NOSKEL)) {
		oputs("for f in INPUT OUTPUT FORWARD; do ipchains -P $f DENY; done");
		oputs("ipchains -F; ipchains -X");
		oputs("ipchains -N forw_out");
	}
	if((r = filtergen_cprod(filter, &cb_ipchains, &misc)) < 0)
		return r;
	if(!(flags & FF_NOSKEL)) {
		oputs("for f in INPUT OUTPUT FORWARD forw_out; do ipchains -A $f -l -j DENY; done");
		r += 3;
	}
	return r;
}
