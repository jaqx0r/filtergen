/*
 * Filter generator, ipchains driver
 *
 * XXX - maybe some of this could be shared with the iptables one?
 *
 * $Id: fg-ipchains.c,v 1.19 2002/07/31 22:17:21 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "filter.h"
#include "util.h"


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
