/*
 * Filter generator, iptables driver
 *
 * $Id: fg-iptables.c,v 1.24 2002/07/19 13:03:28 matthew Exp $
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
 * 4. Reject rules need to go into FORWARD as well as
 *    INPUT or OUTPUT.
 * 5. INPUT rules also get added to FORWARD.
 * 6. If LOCALONLY is set, no FORWARD rules are output.
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
#define	A_TCP	0x10
#define	A_UDP	0x20

static int cb_iptables_rule(const struct filterent *ent, struct fg_misc *misc)
{
	char *natchain = NULL, *rulechain = NULL, *revchain = NULL;
	char *natrule = NULL, *rule = NULL, *rule_r = NULL;
	int neednat = 0, needret = 0;
	int islocal = (ent->rtype != ROUTEDONLY);
	int isforward = (ent->rtype != LOCALONLY);
	long *feat = (long*)misc->misc;
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
		*feat |= A_TCP;
		APPSS2(natrule, "-p", "tcp");
		APPSS2(rule, "-p", "tcp");
		APPSS2(rule_r, "-p", "tcp");
		APPS(rule, "-m state --state NEW,ESTABLISHED");
		APPS(rule_r, "-m state --state ESTABLISHED ! --syn");
		break;
	case UDP:
		needret++;
		*feat |= A_UDP;
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
	default:;
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
		if(islocal) oprintf("iptables -A %s %s LOG\n", rulechain, rule+1);
		if(isforward) oprintf("iptables -A FORWARD %s LOG\n", rule+1);
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

	if((misc->flags & FF_LSTATE) && (target != F_REJECT)) needret = 0;

	if(neednat) oprintf("iptables -t nat -A %s %s\n", natchain, natrule+1);
	if(islocal) oprintf("iptables -A %s %s\n", rulechain, rule+1);
	if(needret) oprintf("iptables -A %s %s\n", revchain, rule_r+1);
	if(isforward) {
		 oprintf("iptables -A FORWARD %s\n", rule+1);
		 if(needret) oprintf("iptables -A FORWARD %s\n", rule_r+1);
	}

	free(natrule); free(rule); free(rule_r);
	return 1 + !!needret + !!neednat;
}

static int cb_iptables_group(const char *name)
{
	oprintf("iptables -N %s\n", name);
	return 1;
}

int fg_iptables(struct filter *filter, int flags)
{
	long feat = 0;
	int r;
	struct fg_misc misc = { flags, &feat };
	fg_callback cb_iptables = {
	rule:	cb_iptables_rule,
	group:	cb_iptables_group,
	};
	const int nchains = 3;

	filter_unroll(&filter);

	if(!(flags & FF_NOSKEL)) {
		oputs("CHAINS=\"INPUT OUTPUT FORWARD\"");
		oputs("");

		oputs("# Flush/remove rules, set policy");
		oputs("for f in $CHAINS; do iptables -P $f DROP; done");
		oputs("iptables -F; iptables -X");
		oputs("iptables -t nat -F; iptables -t nat -X");
		oputs("");

		oputs("# Setup INVALID chain");
		oputs("iptables -N INVALID");
		oputs("iptables -A INVALID -j LOG --log-prefix \"invalid \"");
		oputs("iptables -A INVALID -j DROP");
		oputs("for f in $CHAINS; do\n"
		     "\tiptables -I $f -m state --state INVALID -j INVALID;\n"
		     "done");
		oputs("");
		r += nchains;
	}
	if((r = filtergen_cprod(filter, &cb_iptables, &misc)) < 0)
		return r;
	if(!(flags & FF_NOSKEL)) {
		if((flags & FF_LSTATE) && (feat & (A_TCP|A_UDP))) {
			oputs("for f in $CHAINS; do");
			if(feat & A_TCP) {
				r += nchains;
				oputs("\tiptables -I $f -p tcp ! --syn -m state --state ESTABLISHED -j ACCEPT;");
			}
			if(feat & A_UDP) {
				r += nchains;
				oputs("\tiptables -I $f -p udp -m state --state ESTABLISHED -j ACCEPT;");
			}
			oputs("done");
		}
		oputs("for f in $CHAINS; do iptables -A $f -j LOG; done");
		r += nchains;
	}
	return r;
}
