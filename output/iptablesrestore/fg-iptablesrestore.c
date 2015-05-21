/* filter generator, iptables-restore driver
 *
 * Copyright (c) 2002 Matthew Kirkwood
 * 		 2006 Oliver Hookins
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Description of generated filter
 *
 * 1. Policy:
 *    + "filter" chains DROP.
 *    + "nat" chains ACCEPT.
 * 2. State:
 *    + Non-TCP/UDP rules don't include state.
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

/* a simple linked list for the nat table */
struct llnode {
	char *data;
	struct llnode *next;
};

/* head of the list and the current node */
struct llnode *head = NULL;
struct llnode *curr = NULL;

/* addnode: add another list node to the end of the llist */
static void addnode(const char *data_in) {
	struct llnode *tmp;
	
	if ((tmp = malloc(sizeof(*tmp))) == NULL) {
		fprintf(stderr, "unable to allocate memory for nat table processing\n"); abort();
        }

	tmp->data = strdup(data_in);
	tmp->next = NULL;
	
	if (head == NULL) {
		head = curr = tmp;
		curr->next = NULL;
	}
	else {
		curr->next = tmp;
		tmp->next = NULL;
	}
	
	curr = tmp;	
}

/* freelist: free the memory allocations used by the llist */
static void freelist() {
	struct llnode *tmp;

	for(curr = tmp = head; curr != NULL; tmp = curr) {
		curr = curr->next;
		free(tmp);
	}
}

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


static int cb_iptablesrestore_rule(const struct filterent *ent, struct fg_misc *misc)
{
    char *rulechain = NULL, *revchain = NULL, *natchain = NULL;
    char *ruletarget = NULL, *revtarget = NULL, *nattarget = NULL;
    char *natrule = NULL, *rule = NULL, *rule_r = NULL;
    char *forchain = NULL, *forrevchain = NULL;
    char *fortarget = NULL, *forrevtarget = NULL;
    char *subchain = NULL, *subtarget = NULL;
    int neednat = 0, needret = 0;
    int islocal = (ent->rtype != ROUTEDONLY);
    int isforward = (ent->rtype != LOCALONLY);
    long *feat = (long*)misc->misc;
    enum filtertype target = ent->target;
    int orules = 0;

    /* nat rule? */
    if((target == MASQ) || (target == REDIRECT)) {
	neednat = 1;
	if((target == MASQ) && (ent->direction == INPUT)) {
	    fprintf(stderr, "can't masquerade on input\n");
	    return -1;
	} else if((target == REDIRECT) && (ent->direction == OUTPUT)) {
	    fprintf(stderr, "can't redirect on output\n");
	    return -1;
	}
    }

    /* sub-stuff? */
    if(target == F_SUBGROUP) {
	subtarget = strapp(strdup(ent->subgroup), "-");
	needret = 1;
    } else subtarget = strdup("");

    if(ent->groupname) subchain = strapp(strdup(ent->groupname), "-");
    else subchain = strdup("");

    switch(ent->direction) {
      case INPUT:
	natchain = strdup("PREROUTING");
	rulechain = strdup("INPUT");
	revchain = strdup("OUTPUT");
	forchain = strdup("FORWARD");
	forrevchain = strdup("FORW_OUT");
	if(ent->iface) {
	    if(NEG(DIRECTION)) {
		APPS(natrule, "!");
		APPS(rule, "!");
		APPS(rule_r, "!");
	    }
	    APPSS2(natrule, "-i", ent->iface);
	    APPSS2(rule, "-i", ent->iface);
	    APPSS2(rule_r, "-o", ent->iface);
	}
	break;
      case OUTPUT:
	natchain = strdup("POSTROUTING");
	rulechain = strdup("OUTPUT");
	revchain = strdup("INPUT");
	forchain = strdup("FORW_OUT");
	forrevchain = strdup("FORWARD");
	if(ent->iface) {
	    if(NEG(DIRECTION)) {
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
    /* FIXME: state established on reverse for every rule, not just
     * specifically udp and tcp */
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
    if(ESET(ent, LOG)) {
	char *lc, *la, *ls;
	if(ent->logmsg) {
	    lc = strdup(" --log-prefix \"");
	    la = ent->logmsg;
	    ls = strdup(" \"");
	} else
	    lc = la = ls = strdup("");
	if(islocal) orules++,oprintf("-A %s %s LOG%s%s%s\n", rulechain, rule+1, lc, la, ls);
	if(isforward) orules++,oprintf("-A %s %s LOG%s%s%s\n", forchain, rule+1, lc, la, ls);
    }

    /* Do this twice, once for NAT, once for filter */
    if(neednat) {
	switch(target) {
	  case MASQ:
	    nattarget = strdup("MASQUERADE");
	    break;
	  case REDIRECT:
	    nattarget = strdup("REDIRECT");
	    break;
	  default:
	    abort();
	}
    }

    switch(target) {
      case MASQ:
      case REDIRECT:
      case T_ACCEPT:
	ruletarget = revtarget =
	    fortarget = forrevtarget = strdup("ACCEPT");
	switch(ent->direction) {
	  case INPUT:
	    fortarget = strdup("FORW_OUT");
	    break;
	  case OUTPUT:
	    forrevtarget = strdup("FORW_OUT");
	    break;
	  default:
	    abort();
	}
	break;
      case DROP:
	ruletarget = fortarget = strdup("DROP");
	switch (ent->direction) {
	  case INPUT:
	    fortarget = strdup("FORW_OUT");
	    break;
	  case OUTPUT:
	    /* nothing, fortarget is already DROP */
	    break;
	  default:
	    abort();
	}
	needret = 0;
	break;
      case T_REJECT:
	ruletarget = fortarget = strdup("REJECT");
	switch (ent->direction) {
	  case INPUT:
	    fortarget = strdup("FORW_OUT");
	    break;
	  case OUTPUT:
	    /* nothing, fortarget is already DROP */
	    break;
	  default:
	    abort();
	}
	needret = 0;
	*feat |= T_REJECT;
	break;
      case F_SUBGROUP:
	switch(ent->direction) {
	  case INPUT:
	    ruletarget = strdup("INPUT");
	    revtarget = strdup("OUTPUT");
	    fortarget = strdup("FORWARD");
	    forrevtarget = strdup("FORW_OUT");
	    break;
	  case OUTPUT:
	    ruletarget = strdup("OUTPUT");
	    revtarget = strdup("INPUT");
	    fortarget = strdup("FORW_OUT");
	    forrevtarget = strdup("FORWARD");
	    break;
	  default:
	    abort();
	}
	break;
      default:
	abort();
    }

    if((misc->flags & FF_LSTATE) && (target != T_REJECT)) needret = 0;

    if(ent->oneway) needret = 0;

    if(neednat) {
	char tmp[1000];
	snprintf(tmp, (sizeof(tmp) - 1), "-A %s%s %s %s%s", subchain, natchain, natrule+1, subtarget, nattarget);
	addnode(tmp);
	orules++;
    }
    if(islocal) orules++,oprintf("-A %s%s %s %s%s\n", subchain, rulechain, rule+1, subtarget, ruletarget);
    if(needret) orules++,oprintf("-I %s%s %s %s%s\n", subchain, revchain, rule_r+1, subtarget, revtarget);
    if(isforward) {
	orules++,oprintf("-A %s%s %s %s%s\n", subchain, forchain, rule+1, subtarget, fortarget);
	if(needret) orules++,oprintf("-I %s%s %s %s%s\n", subchain, forrevchain, rule_r+1, subtarget, forrevtarget);
    }

    free(natrule); free(rule); free(rule_r);
    free(subchain); free(subtarget);
    return orules;
}

static int cb_iptablesrestore_group(const char *name)
{
    oprintf("-N %s-INPUT\n", name);
    oprintf("-N %s-OUTPUT\n", name);
    oprintf("-N %s-FORWARD\n", name);
    oprintf("-N %s-FORW_OUT\n", name);
    return 4;
}

int fg_iptablesrestore(struct filter *filter, int flags)
{
    long feat = 0;
    int r = 0;
    struct fg_misc misc = { flags, &feat };
    fg_callback cb_iptablesrestore = {
	rule:	cb_iptablesrestore_rule,
	group:	cb_iptablesrestore_group,
    };
    const int nchains = 3;

    filter_unroll(&filter);
    filter_apply_flags(filter, flags);

    if(!(flags & FF_NOSKEL)) {
	/* Add the nat table headers to the llist, to be
 	 * followed up by optional real rules
 	 */
	addnode("*nat");
	addnode(":PREROUTING ACCEPT [0:0]");
	addnode(":POSTROUTING ACCEPT [0:0]");
	addnode(":OUTPUT ACCEPT [0:0]");
	/* No COMMIT, that is done after rule output */

	oputs("*filter");
	oputs(":FORW_OUT - [0:0]");
	oputs(":INPUT DROP [0:0]");
	oputs(":FORWARD DROP [0:0]");
	oputs(":INVALID - [0:0]");
	oputs(":OUTPUT DROP [0:0]");

#if 0
	oputs("-A INVALID -j LOG --log-prefix \"invalid \"");
#endif
	oputs("-A INVALID -j DROP");
	oputs("-A INPUT -m state --state INVALID -j INVALID");
	oputs("-A OUTPUT -m state --state INVALID -j INVALID");
	oputs("-A FORWARD -m state --state INVALID -j INVALID");
	r += nchains;
    }
    if((r = filtergen_cprod(filter, &cb_iptablesrestore, &misc)) < 0)
	return r;
    if(!(flags & FF_NOSKEL)) {
	if((flags & FF_LSTATE) && (feat & (A_TCP|A_UDP))) {
	    if(feat & A_TCP) {
		r += nchains;
		oputs("-I INPUT -p tcp ! --syn -m state --state ESTABLISHED -j ACCEPT");
		oputs("-I OUTPUT -p tcp ! --syn -m state --state ESTABLISHED -j ACCEPT");
		oputs("-I FORWARD -p tcp ! --syn -m state --state ESTABLISHED -j ACCEPT");
	    }
	    if(feat & A_UDP) {
		r += nchains;
		oputs("-I INPUT -p udp -m state --state ESTABLISHED -j ACCEPT");
		oputs("-I OUTPUT -p udp -m state --state ESTABLISHED -j ACCEPT");
		oputs("-I FORWARD -p udp -m state --state ESTABLISHED -j ACCEPT");
	    }
	}
#if 0
	oputs("-A INPUT -j LOG");
	oputs("-A OUTPUT -j LOG");
	oputs("-A FORWARD -j LOG");
	r += nchains;
#endif
    }
    oputs("COMMIT");
    
    /* Dump contents of the llist which contains the NAT tabl rules */
    for(curr = head; curr != NULL; curr = curr->next)
	oputs(curr->data);

    oputs("COMMIT");
    freelist();

    return r;
}


/* Rules which just flush the packet filter */
int flush_iptablesrestore(enum filtertype policy) {
    char * ostr;

    // FIXME: Should the nat table always have a default ACCEPT policy?
    oputs("*nat");
    oputs(":PREROUTING ACCEPT [0:0]");
    oputs(":POSTROUTING ACCEPT [0:0]");
    oputs(":OUTPUT ACCEPT [0:0]");
    oputs("COMMIT");

    // filter table
    oputs("*filter");

    switch (policy) {
      case T_ACCEPT:
	ostr = strdup("ACCEPT");
	break;
      case DROP:
	ostr = strdup("DROP");
	break;
      case T_REJECT:
	ostr = strdup("REJECT");
	break;
      default:
	fprintf(stderr, "invalid filtertype %d\n", policy);
	abort();
    }
    oprintf(":INPUT %s [0:0]\n", ostr);
    oprintf(":OUTPUT %s [0:0]\n", ostr);
    oprintf(":FORWARD %s [0:0]\n", ostr);
    oputs("COMMIT");

    return 0;
}
