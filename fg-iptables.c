/*
 * Filter generator, iptables driver
 *
 * $Id: fg-iptables.c,v 1.30 2002/08/20 22:54:38 matthew Exp $
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


static char *appip(char *r, const struct addr_spec *h)
{
	APPS(r, h->addrstr);
	if(h->maskstr) APP2(r, "/", h->maskstr);
	return r;
}
#define	APPIP(r, h)		(r = appip(r, h))
#define	APPIP2(f, r, h)		(APPS(r, f), APPIP(r, h))


static char *appport(char *r, const struct port_spec *h)
{
	APPS(r, h->minstr);
	if(h->maxstr) APP2(r, ":", h->maxstr);
	return r;
}
#define	APPPORT(r, h)		(r = appport(r, h))
#define	APPPORT2(f, r, h)	(APPS(r, f), APPPORT(r, h))


static int cb_iptables_rule(const struct filterent *ent, struct fg_misc *misc)
{
	char *rulechain = NULL, *revchain = NULL, *natchain = NULL;
	char *ruletarget = NULL, *revtarget = NULL, *nattarget = NULL;
	char *natrule = NULL, *rule = NULL, *rule_r = NULL;
	char *forchain = NULL, *forrevchain = NULL;
	char *fortarget = NULL, *forrevtarget = NULL;
	int neednat = 0, needret = 0;
	int islocal = (ent->rtype != ROUTEDONLY);
	int isforward = (ent->rtype != LOCALONLY);
	long *feat = (long*)misc->misc;
	enum filtertype target = ent->target;
	int orules = 0;

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
			forchain = "FORWARD";
			forrevchain = "FORW_OUT";
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
			forchain = "FORW_OUT";
			forrevchain = "FORWARD";
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
	if(ent->proto.name) {
		int needstate = 0;

		APPSS2(natrule, "-p", ent->proto.name);
		APPSS2(rule, "-p", ent->proto.name);
		APPSS2(rule_r, "-p", ent->proto.name);
		switch(ent->proto.num) {
		case IPPROTO_TCP:
			needret++;
			needstate++;
			*feat |= A_TCP;
			APPS(rule_r, "! --syn");
			break;
		case IPPROTO_UDP:
			needret++;
			needstate++;
			*feat |= A_UDP;
			break;
		}
		if(needstate) {
			APPS(rule, "-m state --state NEW,ESTABLISHED");
			APPS(rule_r, "-m state --state ESTABLISHED");
		}
	}

	if(ent->srcaddr.addrstr) {
		NEGA(natrule, SOURCE); NEGA(rule, SOURCE); NEGA(rule_r, SOURCE);
		APPIP2("-s", natrule, &ent->srcaddr);
		APPIP2("-s", rule, &ent->srcaddr);
		APPIP2("-d", rule_r, &ent->srcaddr);
	}
	if(ent->dstaddr.addrstr) {
		NEGA(natrule, DEST); NEGA(rule, DEST); NEGA(rule_r, DEST);
		APPIP2("-d", natrule, &ent->dstaddr);
		APPIP2("-d", rule, &ent->dstaddr);
		APPIP2("-s", rule_r, &ent->dstaddr);
	}

	switch(ent->proto.num) {
	case 0: break;
	case IPPROTO_UDP: case IPPROTO_TCP:
		if(ent->u.ports.src.minstr) {
			NEGA(natrule, SPORT);
			NEGA(rule, SPORT);
			NEGA(rule_r, SPORT);
			APPPORT2("--sport", natrule, &ent->u.ports.src);
			APPPORT2("--sport", rule, &ent->u.ports.src);
			APPPORT2("--dport", rule_r, &ent->u.ports.src);
		}
		if(ent->u.ports.dst.minstr) {
			NEGA(natrule, DPORT);
			NEGA(rule, DPORT);
			NEGA(rule_r, DPORT);
			APPPORT2("--dport", natrule, &ent->u.ports.dst);
			APPPORT2("--dport", rule, &ent->u.ports.dst);
			APPPORT2("--sport", rule_r, &ent->u.ports.dst);
		}
		break;
	case IPPROTO_ICMP:
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
		if(islocal) orules++,oprintf("iptables -A %s %s LOG\n", rulechain, rule+1);
		if(isforward) orules++,oprintf("iptables -A %s %s LOG\n", forchain, rule+1);
	}

	/* Do this twice, once for NAT, once for filter */
	if(neednat) {
		switch(target) {
		case F_MASQ:	nattarget = "MASQUERADE"; break;
		case F_REDIRECT:nattarget = "REDIRECT"; break;
		default: abort();
		}
	}

	switch(target) {
	case F_MASQ: case F_REDIRECT:
	case F_ACCEPT:	ruletarget = revtarget = fortarget = forrevtarget = "ACCEPT";
			switch(ent->direction) {
			case F_INPUT: fortarget = "FORW_OUT"; break;
			case F_OUTPUT: forrevtarget = "FORW_OUT"; break;
			default: abort();
			}
			break;
	case F_DROP:	ruletarget = fortarget = "DROP"; needret = 0; break;
	case F_REJECT:	ruletarget = fortarget = "REJECT"; needret = 0;
			*feat |= F_REJECT; break;
	case F_SUBGROUP:ruletarget = revtarget = ent->subgroup; break;
	default: abort();
	}

	if((misc->flags & FF_LSTATE) && (target != F_REJECT)) needret = 0;

	if(neednat) orules++,oprintf("iptables -t nat -A %s %s %s\n", natchain, natrule+1, nattarget);
	if(islocal) orules++,oprintf("iptables -A %s %s %s\n", rulechain, rule+1, ruletarget);
	if(needret) orules++,oprintf("iptables -I %s %s %s\n", revchain, rule_r+1, revtarget);
	if(isforward) {
		 orules++,oprintf("iptables -A %s %s %s\n", forchain, rule+1, fortarget);
		 if(needret) orules++,oprintf("iptables -I %s %s %s\n", forrevchain, rule_r+1, forrevtarget);
	}

	free(natrule); free(rule); free(rule_r);
	return orules;
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
	filter_apply_flags(filter, flags);

	if(!(flags & FF_NOSKEL)) {
		oputs("CHAINS=\"INPUT OUTPUT FORWARD\"");
		oputs("");

		oputs("# Flush/remove rules, set policy");
		oputs("for f in $CHAINS; do iptables -P $f DROP; done");
		oputs("iptables -F; iptables -X");
		oputs("iptables -t nat -F; iptables -t nat -X");
		oputs("");

		oputs("# Create FORW_OUT chain");
		oputs("iptables -N FORW_OUT");
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
