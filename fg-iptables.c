/*
 * Filter generator, iptables driver
 *
 * $Id: fg-iptables.c,v 1.10 2001/10/06 18:25:16 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"


static int cb_iptables(const struct filterent *ent, void *misc)
{
	char *rule = NULL, *rule_r = NULL;
	int needret = 0;

	APP(rule, "iptables");
	APP(rule_r, "iptables");

	/* nat rule */
	if((ent->target == F_MASQ) || (ent->target == F_REDIRECT)) {
		if((ent->target == F_MASQ) && (ent->direction == F_OUTPUT)) {
			fprintf(stderr, "can't masquerade on input\n");
			return -1;
		} else if((ent->target == F_MASQ) && (ent->direction == F_OUTPUT)) {
			fprintf(stderr, "can't redirect on output\n");
			return -1;
		}
		APPSS2(rule, "-t", "nat");
		/* NB: we don't need NAT for the return packets */
	}

	APPS(rule, "-A");
	APPS(rule_r, "-A");

	switch(ent->direction) {
	case F_INPUT:	APPS(rule, (ent->target == F_REDIRECT) ? "PREROUTING" : "INPUT");
			APPS(rule_r, "OUTPUT");
			if(ent->iface) {
				if(NEG(INPUT)) {
					APPS(rule, "!");
					APPS(rule_r, "!");
				}
				APPSS2(rule, "-i", ent->iface);
				APPSS2(rule_r, "-o", ent->iface);
			}
			break;
	case F_OUTPUT:	APPS(rule, (ent->target == F_MASQ) ? "POSTROUTING" : "OUTPUT");
			APPS(rule_r, "INPUT");
			if(ent->iface) {
				if(NEG(OUTPUT)) {
					APPS(rule, "!");
					APPS(rule_r, "!");
				}
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
		APPSS2(rule, "-p", "tcp");
		APPSS2(rule_r, "-p", "tcp");
		APPS(rule, "-m state --state NEW,ESTABLISHED");
		APPS(rule_r, "-m state --state ESTABLISHED ! --syn");
		break;
	case UDP:
		needret++;
		APPSS2(rule, "-p", "udp");
		APPSS2(rule_r, "-p", "udp");
		APPS(rule, "-m state --state NEW,ESTABLISHED");
		APPS(rule_r, "-m state --state ESTABLISHED");
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
	default:
	}

	APPS(rule, "-j"); APPS(rule_r, "-j");

	/* Yuck, separate rules for logging packets.  Be still my
	 * beating lunch.
	 *
	 * Logging and target rules have to be the last bits
	 * before output, or this doesn't work.  This will also
	 * fail if any mangling has been done above.
	 */
	if(ent->log) {
		char *logrule = strdup(rule);
		APPS(logrule, "LOG");
		puts(logrule);
	}

	switch(ent->target) {
	case F_ACCEPT:	APPS(rule, "ACCEPT");
			APPS(rule_r, "ACCEPT"); break;
	case F_DROP:	APPS(rule, "DROP"); needret = 0; break;
	case F_REJECT:	APPS(rule, "REJECT"); needret = 0; break;
	case F_MASQ:	APPS(rule, "MASQUERADE");
			APPS(rule_r, "ACCEPT"); break;
	case F_REDIRECT:APPS(rule, "REDIRECT");
			APPS(rule_r, "ACCEPT"); break;
	default: abort();
	}

	puts(rule);
	if(needret) puts(rule_r);

	free(rule); free(rule_r);
	return 1 + !!needret;
}


int fg_iptables(struct filter *filter)
{
	filter_unroll(&filter);
	return filtergen_cprod(filter, cb_iptables, NULL);
}
