/*
 * Filter generator, ipchains driver
 *
 * XXX - maybe some of this could be shared with the iptables one?
 *
 * $Id: fg-ipchains.c,v 1.14 2002/07/18 21:23:19 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "filter.h"
#include "util.h"


static int cb_ipchains_rule(const struct filterent *ent, struct fg_misc *misc)
{
	char *rule = NULL, *rule_r = NULL;
	char *rulechain = NULL, *revchain = NULL;
	int needret = 0;

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
	case F_INPUT:	rulechain = "input"; revchain = "output"; break;
	case F_OUTPUT:	rulechain = (ent->target == F_MASQ)
						? "forward" : "output";
			revchain = "input"; break;
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
	switch(ent->proto) {
	case 0:	break;
	case TCP:
		needret++;
		APPSS2(rule, "-p", "tcp");
		APPSS2(rule_r, "-p", "tcp");
		APPS(rule_r, "! --syn");
		break;
	case UDP:
		needret++;
		APPSS2(rule, "-p", "udp");
		APPSS2(rule_r, "-p", "udp");
		break;
	case ICMP:
		APPSS2(rule, "-p", "icmp");
		break;
	default: abort();
	}

	if(ent->srcaddr) {
		NEGA(rule, SOURCE); NEGA(rule_r, SOURCE);
		APPSS2(rule, "-s", ent->srcaddr);
		APPSS2(rule_r, "-d", ent->srcaddr);
	}
	if(ent->dstaddr) {
		NEGA(rule, DEST); NEGA(rule_r, DEST);
		APPSS2(rule, "-d", ent->dstaddr);
		APPSS2(rule_r, "-s", ent->dstaddr);
	}

	switch(ent->proto) {
	case 0: break;
	case UDP: case TCP:
		if(ent->u.ports.src) {
			NEGA(rule, SPORT); NEGA(rule_r, SPORT);
			APPSS2(rule, "--sport", ent->u.ports.src);
			APPSS2(rule_r, "--dport", ent->u.ports.src);
		}
		if(ent->u.ports.dst) {
			NEGA(rule, DPORT); NEGA(rule_r, DPORT);
			APPSS2(rule, "--dport", ent->u.ports.dst);
			APPSS2(rule_r, "--sport", ent->u.ports.dst);
		}
		break;
	case ICMP:
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
	case F_ACCEPT:	APPS(rule, "ACCEPT"); APPS(rule_r, "ACCEPT"); break;
	case F_DROP:	APPS(rule, "DENY"); needret = 0; break;
	case F_REJECT:	APPS(rule, "REJECT"); needret = 0; break;
	case F_MASQ:	APPS(rule, "MASQ"); APPS(rule_r, "ACCEPT"); break;
	case F_REDIRECT:APPS(rule, "REDIRECT"); APPS(rule_r, "ACCEPT"); break;
	case F_SUBGROUP:APPS(rule, ent->subgroup); APPS(rule_r, ent->subgroup); break;
	default: abort();
	}

	oprintf("ipchains -A %s %s\n", rulechain, rule+1);
	if(needret) oprintf("ipchains -A %s %s\n", revchain, rule_r+1);

	free(rule); free(rule_r);
	return 1 + !!needret;
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
	if(!(flags & FF_NOSKEL)) {
		oputs("for f in INPUT OUTPUT FORWARD; do ipchains -P $f DENY; done");
		oputs("ipchains -F; ipchains -X");
	}
	if((r = filtergen_cprod(filter, &cb_ipchains, &misc)) < 0)
		return r;
	if(!(flags & FF_NOSKEL)) {
		oputs("for f in INPUT OUTPUT FORWARD; do ipchains -A $f -l -j DENY; done");
		r += 3;
	}
	return r;
}
