/* filter generator, ipchains driver
 *
 * Copyright (c) 2002 Matthew Kirkwood
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
 * XXX - maybe some of this could be shared with the iptables one?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

/* full path to ipchains executable */
#define IPCHAINS "/sbin/ipchains"


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


static int cb_ipchains_rule(const struct filterent *ent, struct fg_misc *misc __attribute__((unused)))
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
      case INPUT:
	rulechain = strdup("input");
	revchain = strdup("output");
	forchain = strdup("forward");
	forrevchain = strdup("forw_out");
	break;
      case OUTPUT:
	rulechain = (ent->target == MASQ) ? strdup("forw_out") : strdup("output") ;
	    revchain = strdup("input");
	    forchain = strdup("forw_out");
	    forrevchain = strdup("forward");
	    break;
      default:
	fprintf(stderr, "unknown direction\n");
	abort();
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
      case T_ACCEPT:
	ruletarget = revtarget =
	    fortarget = forrevtarget = strdup("ACCEPT");
	switch(ent->direction) {
	  case INPUT:
	    fortarget = strdup("forw_out");
	    break;
	  case OUTPUT:
	    forrevtarget = strdup("forw_out");
	    break;
	  default:
	    abort();
	}
	break;
      case DROP:
	ruletarget = fortarget = strdup("DENY");
	needret = 0;
	break;
      case T_REJECT:
	ruletarget = fortarget = strdup("REJECT");
	needret = 0;
	break;
      case MASQ:
	ruletarget = strdup("MASQ");
	revtarget = strdup("ACCEPT");
	break;
      case REDIRECT:
	ruletarget = strdup("REDIRECT");
	revtarget = strdup("ACCEPT");
	break;
      case F_SUBGROUP:
	switch(ent->direction) {
	  case INPUT:
	    ruletarget = strdup("input");
	    revtarget = strdup("output");
	    fortarget = strdup("forward");
	    forrevtarget = strdup("forw_out");
	    break;
	  case OUTPUT:
	    ruletarget = strdup("output");
	    revtarget = strdup("input");
	    fortarget = strdup("forw_out");
	    forrevtarget = strdup("forward");
	    break;
	  default:
	    abort();
	}
	break;
      default:
	abort();
    }

    if(ent->oneway) needret = 0;

    orules++, oprintf(IPCHAINS" -A %s%s %s %s%s\n", subchain, rulechain, rule+1, subtarget, ruletarget);
    if(needret) orules++, oprintf(IPCHAINS" -I %s%s %s %s%s\n", subchain, revchain, rule_r+1, subtarget, revtarget);
    if(isforward) {
	orules++, oprintf(IPCHAINS" -A %s%s %s %s%s\n", subchain, forchain, rule+1, subtarget, fortarget);
	if(needret) orules++, oprintf(IPCHAINS" -I %s%s %s %s%s\n", subchain, forrevchain, rule_r+1, subtarget, forrevtarget);
    }

    free(rule); free(rule_r);
    free(subchain); free(subtarget);
    return orules;
}

static int cb_ipchains_group(const char *name)
{
    oprintf("for f in input output forward forw_out; do "IPCHAINS" -N %s-${f}; done\n", name);
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
	oputs("for f in INPUT OUTPUT FORWARD; do "IPCHAINS" -P $f DENY; done");
	oputs(IPCHAINS" -F; "IPCHAINS" -X");
	oputs(IPCHAINS" -N forw_out");
    }
    if((r = filtergen_cprod(filter, &cb_ipchains, &misc)) < 0)
	return r;
#if 0
    if(!(flags & FF_NOSKEL)) {
	oputs("for f in INPUT OUTPUT FORWARD forw_out; do "IPCHAINS" -A $f -l -j DENY; done");
	r += 3;
    }
#endif
    return r;
}


/* Rules which just flush the packet filter */
int flush_ipchains(enum filtertype policy)
{
    char *ostr;
    oputs("CHAINS=\"INPUT OUTPUT FORWARD\"");
    oputs("");

    switch(policy) {
      case T_ACCEPT:
	ostr = strdup("ACCEPT");
	break;
      case DROP:
	ostr = strdup("DENY");
	break;
      case T_REJECT:
	ostr = strdup("REJECT");
	break;
      default:
	fprintf(stderr, "invalid filtertype %d\n", policy);
	abort();
    }
    oprintf("for f in $CHAINS; do "IPCHAINS" -P $f %s; done\n", ostr);
    oputs(IPCHAINS" -F; "IPCHAINS" -X");

    return 0;
}
