/*
 * Filter generator, ipchains driver
 *
 * XXX - maybe this should be shared with the ipchains one?
 *
 * $Id: fg-ipchains.c,v 1.2 2001/10/03 20:17:13 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "filter.h"
#include "util.h"


int cb_ipchains(const struct filterent *ent, void *misc)
{
	char *rule = NULL, *rule_r = NULL;
	int needret = 0;

	APP(rule, "ipchains -A");
	APP(rule_r, "ipchains -A");

	/* this should all be put in a table somehow */

	switch(ent->direction) {
	case F_INPUT:	APPS(rule, "input"); APPS(rule_r, "output"); break;
	case F_OUTPUT:	APPS(rule, "output"); APPS(rule_r, "input"); break;
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
			APPS2(rule, "--sport=", ent->u.ports.src);
			APPS2(rule_r, "--dport=", ent->u.ports.src);
		}
		if(ent->u.ports.dst) {
			NEGA(rule, DPORT); NEGA(rule_r, DPORT);
			APPS2(rule, "--dport=", ent->u.ports.dst);
			APPS2(rule_r, "--sport=", ent->u.ports.dst);
		}
		break;
	default:
	}

	switch(ent->target) {
	case F_ACCEPT:	APPSS2(rule, "-j", "ACCEPT");
			APPSS2(rule_r, "-j", "ACCEPT"); break;
	case F_DROP:	APPSS2(rule, "-j", "DROP"); 
			APPSS2(rule_r, "-j", "DROP"); break;
	case F_REJECT:	APPSS2(rule, "-j", "REJECT"); needret = 0; break;
	default: abort();
	}

	puts(rule);
	if(needret) puts(rule_r);

	free(rule); free(rule_r);
	return 1 + !!needret;
}


int fg_ipchains(struct filter *filter)
{
	filter_unroll(&filter);
	return filtergen_cprod(filter, cb_ipchains, NULL);
}
