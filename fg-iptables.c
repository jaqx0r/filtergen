/*
 * Filter generator, iptables driver
 *
 * $Id: fg-iptables.c,v 1.16 2002/04/14 11:38:40 matthew Exp $
 */

/*
 * Description of generated filter
 *
 * 1. Policy:
 *    + "filter" chains DROP.
 *    + "nat" chains ACCEPT.
 * 2. State:
 *    + Allow rules include state.
 *    + Deny rules don't.
 * 3. NAT is done in PRE- and POSTROUTING, as required.
 *    However, we also must add rules to INPUT (for
 *    transproxy) and OUTPUT (for masquerading).
 * 5. Reject rules need to go into FORWARD as well as
 *    INPUT or OUTPUT.
 * 6. Other than for Reject rules, FORWARD stays empty
 *    for now.  The language needs to be extended to
 *    be able to say "please forward this one" - it
 *    shouldn't be default.
 *
 * The above means that each rulechain can generate up to
 * three rules -- nat, in and out.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

/* bitfields for features used */
#define	REJECT	0x01

static int cb_iptables_rule(const struct filterent *ent, void *misc)
{
	char *natchain = NULL, *rulechain = NULL, *revchain = NULL;
	char *natrule = NULL, *rule = NULL, *rule_r = NULL;
	int neednat = 0, needret = 0;
	long *feat = (long*)misc;
	enum filtertype target = ent->target;

	/* nat rule? */
	if((target == F_MASQ) || (target == F_REDIRECT)) {
		neednat = 1;
		if((target == F_MASQ) && (ent->direction == F_OUTPUT)) {
			fprintf(stderr, "can't masquerade on input\n");
			return -1;
		} else if((target == F_MASQ) && (ent->direction == F_OUTPUT)) {
			fprintf(stderr, "can't redirect on output\n");
			return -1;
		}
	}

	switch(ent->direction) {
	case 0:		/* This is a rule in a subgroup */
			revchain = rulechain = ent->groupname;
			break;
	case F_INPUT:	natchain = "PREROUTING";
			rulechain = "INPUT";
			revchain = "OUTPUT";
			if(ent->iface) {
				if(NEG(INPUT)) {
					APPS(natrule, "!");
					APPS(rule, "!");
					APPS(rule_r, "!");
				}
				APPSS2(natrule, "-i", ent->iface);
				APPSS2(rule, "-i", ent->iface);
				APPSS2(rule_r, "-o", ent->iface);
			}
			break;
	case F_OUTPUT:	natchain = "POSTROUTING";
			rulechain = "OUTPUT";
			revchain = "INPUT";
			if(ent->iface) {
				if(NEG(OUTPUT)) {
					APPS(natrule, "!");
					APPS(rule, "!");
					APPS(rule_r, "!");
				}
				APPSS2(natrule, "-o", ent->iface);
				APPSS2(rule, "-o", ent->iface);
				APPSS2(rule_r, "-i", ent->iface);
			}
			break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	/* state and reverse rules here */
	switch(ent->proto) {
	case 0:	break;
	case TCP:
		needret++;
		APPSS2(natrule, "-p", "tcp");
		APPSS2(rule, "-p", "tcp");
		APPSS2(rule_r, "-p", "tcp");
		APPS(rule, "-m state --state NEW,ESTABLISHED");
		APPS(rule_r, "-m state --state ESTABLISHED ! --syn");
		break;
	case UDP:
		needret++;
		APPSS2(natrule, "-p", "udp");
		APPSS2(rule, "-p", "udp");
		APPSS2(rule_r, "-p", "udp");
		APPS(rule, "-m state --state NEW,ESTABLISHED");
		APPS(rule_r, "-m state --state ESTABLISHED");
		break;
	case ICMP:
		APPSS2(natrule, "-p", "icmp");
		APPSS2(rule, "-p", "icmp");
		break;
	default: abort();
	}

	if(ent->srcaddr) {
		NEGA(natrule, SOURCE); NEGA(rule, SOURCE); NEGA(rule_r, SOURCE);
		APPSS2(natrule, "-s", ent->srcaddr);
		APPSS2(rule, "-s", ent->srcaddr);
		APPSS2(rule_r, "-d", ent->srcaddr);
	}
	if(ent->dstaddr) {
		NEGA(natrule, DEST); NEGA(rule, DEST); NEGA(rule_r, DEST);
		APPSS2(natrule, "-d", ent->dstaddr);
		APPSS2(rule, "-d", ent->dstaddr);
		APPSS2(rule_r, "-s", ent->dstaddr);
	}

	switch(ent->proto) {
	case 0: break;
	case UDP: case TCP:
		if(ent->u.ports.src) {
			NEGA(natrule, SPORT);
			APPSS2(natrule, "--sport", ent->u.ports.src);
			NEGA(rule, SPORT);
			APPSS2(rule, "--sport", ent->u.ports.src);
			NEGA(rule_r, SPORT);
			APPSS2(rule_r, "--dport", ent->u.ports.src);
		}
		if(ent->u.ports.dst) {
			NEGA(natrule, DPORT);
			APPSS2(natrule, "--dport", ent->u.ports.dst);
			NEGA(rule, DPORT);
			APPSS2(rule, "--dport", ent->u.ports.dst);
			NEGA(rule_r, DPORT);
			APPSS2(rule_r, "--sport", ent->u.ports.dst);
		}
		break;
	case ICMP:
		if(ent->u.icmp) {
			NEGA(natrule, ICMPTYPE);
			APPSS2(natrule, "--icmp-type", ent->u.icmp);
			NEGA(rule, ICMPTYPE);
			APPSS2(rule, "--icmp-type", ent->u.icmp);
		}
		break;
	default:
	}

	APPS(natrule, "-j"); APPS(rule, "-j"); APPS(rule_r, "-j");

	/* The "rule+1" in the printfs below are an ugly hack to
	 * prevent a double-space in the output rule */

	/* Yuck, separate rules for logging packets.  Be still my
	 * beating lunch.
	 *
	 * Logging and target rules have to be the last bits
	 * before output, or this doesn't work.  This will also
	 * fail if any mangling has been done above.
	 */
	if(ent->log) {
		printf("iptables -A %s %s LOG\n", rulechain, rule+1);
	}

	/* Do this twice, once for NAT, once for filter */
	if(neednat) {
		switch(target) {
		case F_MASQ:	APPS(natrule, "MASQUERADE"); break;
		case F_REDIRECT:APPS(natrule, "REDIRECT"); break;
		default: abort();
		}
	}

	switch(target) {
	case F_MASQ: case F_REDIRECT:
	case F_ACCEPT:	APPS(rule, "ACCEPT");
			APPS(rule_r, "ACCEPT"); break;
	case F_DROP:	APPS(rule, "DROP"); needret = 0; break;
	case F_REJECT:	APPS(rule, "REJECT"); needret = 0;
			free(rule_r); rule_r = strdup(rule);
			revchain = "FORWARD";
			++needret;
			*feat |= F_REJECT; break;
	case F_SUBGROUP:APPS(rule, ent->subgroup);
			APPS(rule_r, ent->subgroup); break;
	default: abort();
	}

	if(neednat) printf("iptables -t nat -A %s %s\n", natchain, natrule+1);
	printf("iptables -A %s %s\n", rulechain, rule+1);
	if(needret) printf("iptables -A %s %s\n", revchain, rule_r+1);

	free(natrule); free(rule); free(rule_r);
	return 1 + !!needret + !!neednat;
}

static int cb_iptables_group(const char *name)
{
	printf("iptables -N %s\n", name);
	return 1;
}

int fg_iptables(struct filter *filter, int flags)
{
	long feat;
	int r;
	fg_callback cb_iptables = {
	rule:	cb_iptables_rule,
	group:	cb_iptables_group,
	};

	filter_unroll(&filter);
	if(!(flags & FF_NOSKEL)) {
		puts("for f in INPUT OUTPUT FORWARD; do iptables -P $f DROP; done");
		puts("iptables -F; iptables -X");
		puts("iptables -t nat -F; iptables -t nat -X");
	}
	if((r = filtergen_cprod(filter, &cb_iptables, (void*)&feat)) < 0)
		return r;
	if(!(flags & FF_NOSKEL)) {
		puts("for f in INPUT OUTPUT FORWARD; do iptables -A $f -j LOG; done");
		r += 3;
	}
	return r;
}
