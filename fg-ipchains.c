/*
 * Filter generator, ipchains driver
 *
 * XXX - maybe some of this could be shared with the iptables one?
 *
 * $Id: fg-ipchains.c,v 1.24 2002/11/11 19:48:38 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char *subchain = NULL, *subtarget = NULL;
	int needret = 0;
	int isforward = (ent->rtype != LOCALONLY);
	int orules = 0;

	/* nat rule */
	if((ent->target == MASQ) || (ent->target == REDIRECT)) {
		if((ent->target == MASQ) && (ent->direction == INPUT)) {
			fprintf(stderr, "can't masquerade on input\n");
			return -1;
		} else if((ent->target == REDIRECT) && (ent->direction == OUTPUT)) {
			fprintf(stderr, "can't redirect on output\n");
			return -1;
		}
	}

	/* sub-stuff? */
	if(ent->target == F_SUBGROUP) {
		subtarget = strapp(strdup(ent->subgroup), "-");
		needret = 1;
	} else
		subtarget = strdup("");

	if(ent->groupname) subchain = strapp(strdup(ent->groupname), "-");
	else subchain = strdup("");

	if(ent->rtype == ROUTEDONLY) {
		fprintf(stderr, "ipchains can't do forward-only rules\n");
		return -1;
	}

	switch(ent->direction) {
	case INPUT:	rulechain = "input"; revchain = "output";
			forchain = "forward"; forrevchain = "forw_out"; break;
	case OUTPUT:	rulechain = (ent->target == MASQ) ? "forw_out" : "output";
			revchain = "input";
			forchain = "forw_out"; forrevchain = "forward"; break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	if(ent->iface) {
		if(NEG(DIRECTION)) {
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

	if(ESET(ent,LOG)) APPS(rule, "-l");

	APPS(rule, "-j"); APPS(rule_r, "-j");

	switch(ent->target) {
	case T_ACCEPT:	ruletarget = revtarget =
			fortarget = forrevtarget = "ACCEPT";
			switch(ent->direction) {
			case INPUT: fortarget = "forw_out"; break;
			case OUTPUT: forrevtarget = "forw_out"; break;
			default: abort();
			}
			break;
	case DROP:	ruletarget = fortarget = "DENY"; needret = 0; break;
	case T_REJECT:	ruletarget = fortarget = "REJECT"; needret = 0; break;
	case MASQ:	ruletarget = "MASQ"; revtarget = "ACCEPT"; break;
	case REDIRECT:	ruletarget = "REDIRECT"; revtarget = "ACCEPT"; break;
	case F_SUBGROUP:switch(ent->direction) {
			case INPUT:	ruletarget = "input";
					revtarget = "output";
					fortarget = "forward";
					forrevtarget = "forw_out";
					break;
			case OUTPUT:	ruletarget = "output";
					revtarget = "input";
					fortarget = "forw_out";
					forrevtarget = "forward";
					break;
			default: abort();
			}
			break;
	default: abort();
	}

	orules++, oprintf("ipchains -A %s%s %s %s%s\n", subchain, rulechain, rule+1, subtarget, ruletarget);
	if(needret) orules++, oprintf("ipchains -I %s%s %s %s%s\n", subchain, revchain, rule_r+1, subtarget, revtarget);
	if(isforward) {
		orules++, oprintf("ipchains -A %s%s %s %s%s\n", subchain, forchain, rule+1, subtarget, fortarget);
		if(needret) orules++, oprintf("ipchains -I %s%s %s %s%s\n", subchain, forrevchain, rule_r+1, subtarget, forrevtarget);
	}

	free(rule); free(rule_r);
	free(subchain); free(subtarget);
	return orules;
}

static int cb_ipchains_group(const char *name)
{
	oprintf("for f in input output forward forw_out; do ipchains -N %s-${f}; done\n", name);
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


/* Rules which just flush the packet filter */
int flush_ipchains(enum filtertype policy, int flags)
{
	char *ostr;
	oputs("CHAINS=\"INPUT OUTPUT FORWARD\"");
	oputs("");

	switch(policy) {
	case T_ACCEPT: ostr = "ACCEPT"; break;
	case DROP: ostr = "DENY"; break;
	case T_REJECT: ostr = "REJECT"; break;
	default:
		fprintf(stderr, "invalid filtertype %d\n", policy);
		abort();
	}
	oprintf("for f in $CHAINS; do ipchains -P $f %s; done\n", ostr);
	oputs("ipchains -F; ipchains -X");

	return 0;
}
