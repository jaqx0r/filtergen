/*
 * Filter generator, iptables driver
 *
 * $Id: fg-iptables.c,v 1.2 2001/10/03 19:01:54 matthew Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "filter.h"


int cb_iptables(const struct filterent *ent, void *misc)
{
	const char *chain, *chain_r, *target;
	const char *proto, *state, *state_r;
	const char *ifarg, *ifarg_r;
	int needret = 0;

	/* this should all be put in a table somehow */

	switch(ent->direction) {
	case F_INPUT:	chain = "input"; chain_r = "output";
			ifarg = "-i"; ifarg_r = "-o"; break;
	case F_OUTPUT:	chain = "output"; chain_r = "input";
			ifarg = "-o"; ifarg_r = "-i"; break;
	default: fprintf(stderr, "unknown direction\n"); abort();
	}

	if(!(ent->iface)) ifarg = ifarg_r = NULL;

	/* state and reverse rules here */
	switch(ent->proto) {
	case 0:
		proto = NULL;
		state = state_r = NULL;
		break;
	case TCP:
		needret++;
		proto = "tcp";
		state = "--state=NEW,ESTABLISHED";
		state_r = "--state=ESTABLISHED ! -y";
		break;
	case UDP:
		needret++;
		proto = "udp";
		state = "--state=NEW,ESTABLISHED";
		state_r = "--state=ESTABLISHED";
		break;
	default: abort();
	}

#define	NL(s) ((s) ?: "")
#define ARGIF(v,s) ((v) ? " " s " " : "")
#define ARGSIF(v,s) ARGIF(v,s), NL(v)
#define NARGSIF(a,v,s) ((ent->whats_negated & (1<<F_##a)) ? " !" : ""),	\
			ARGSIF(v,s)

	switch(ent->target) {
	case F_ACCEPT: target = "ACCEPT"; break;
	case F_DROP: target = "DROP"; break;
	case F_REJECT: needret = 0; state = NULL; target = "REJECT"; break;
	default: abort();
	}

#define FORMAT "iptables -A %s %s %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s %s -J %s\n"
	printf(FORMAT,
			chain,
			NL(ifarg), NL(ent->iface),
			NARGSIF(PROTO, proto, "-p"),
			NARGSIF(SOURCE, ent->srcaddr, "-s"),
			NARGSIF(DEST, ent->dstaddr, "-d"),
			NARGSIF(SPORT, ent->u.ports.src, "--sport"),
			NARGSIF(DPORT, ent->u.ports.dst, "--dport"),
			NL(state), target);
	if(needret) {
		printf(FORMAT,
			chain_r,
			NL(ifarg_r), NL(ent->iface),
			NARGSIF(PROTO, proto, "-p"),
			NARGSIF(SOURCE, ent->srcaddr, "-d"),
			NARGSIF(DEST, ent->dstaddr, "-s"),
			NARGSIF(SPORT, ent->u.ports.src, "--dport"),
			NARGSIF(DPORT, ent->u.ports.dst, "--sport"),
			NL(state_r), target);
	}

	return 1 + !!needret;
}


int fg_iptables(struct filter *filter)
{
	filter_unroll(&filter);
	return filtergen_cprod(filter, cb_iptables, NULL);
}
