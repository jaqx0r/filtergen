/* filter generator, ipfilter driver
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

/* TODO:
 * - does this need skeleton routines?
 * - implement grouping
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

static char *appip(char *r, const struct addr_spec *h)
{
    if(!h->addrstr) return APPS(r, "any");
    APPS(r, h->addrstr);
    if(h->maskstr) APP2(r, "/", h->maskstr);
    return r;
}

static char *appport(char *r, const struct port_spec *p, int neg)
{
    if(!p->minstr) return r;

    APPS(r, "port");
    if(!p->maxstr)
	return APPSS2(r, neg ? "!=" : "=", p->minstr);

    APPS(r, p->minstr);
    APPSS2(r, neg ? "><" : "<>", p->maxstr);
    return r;
}

static char *appicmp(char *r, const char *h, int neg)
{
    if(!h) return r;
    if(neg) APPS(r, "!");
    return APPSS2(r, "icmp-type", h);
}

static int cb_ipfilter_rule(const struct filterent *ent, struct fg_misc *misc __attribute__((unused)))
{
    char *rule = NULL;

    /* target first */
    switch(ent->target) {
      case T_ACCEPT:	APP(rule, "pass"); break;
      case DROP:	APP(rule, "block"); break;
      case T_REJECT:
	/* XXX - can this violate the rule about icmp errors
	 * about icmp errors? */
	if (ent->proto.num == IPPROTO_TCP)
	    APP(rule, "block return-rst");
	else
	    APP(rule, "block return-icmp-as-dest(port-unr)");
	break;
      default: abort();
    }

    /* in or out? */
    switch(ent->direction) {
      case INPUT:	APPS(rule, "in"); break;
      case OUTPUT:	APPS(rule, "out"); break;
      default: fprintf(stderr, "unknown direction\n"); abort();
    }

    if(ESET(ent,LOG)) APPS(rule, "log");

    /* XXX - All our rules have to be "quick", but ipfilter
     * people seem to prefer having defaults at the top.
     * It would be nice to improve the output readability.
     */
    APPS(rule, "quick");

    /* access list name */
    if(ent->iface) APPSS2(rule, "on", ent->iface);

    /* protocol */
    if(ent->proto.name)
	APPSS2(rule, "proto", ent->proto.name);

    APPS(rule, "from");
    rule = appip(rule, &ent->srcaddr);
    rule = appport(rule, &ent->u.ports.src, NEG(SPORT));
    APPS(rule, "to");
    rule = appip(rule, &ent->dstaddr);
    rule = appport(rule, &ent->u.ports.dst, NEG(DPORT));

    rule = appicmp(rule, ent->u.icmp, NEG(ICMPTYPE));

    if(ent->proto.name && (ent->target == T_ACCEPT) && !ent->oneway)
	APPS(rule, "keep state");

    oputs(rule);
    free(rule);
    return 1;
}

int fg_ipfilter(struct filter *filter, int flags)
{
    struct fg_misc misc = { flags, NULL };
    fg_callback cb_ipfilter = {
	rule:	cb_ipfilter_rule, NULL
    };

    filter_nogroup(filter);
    filter_unroll(&filter);
    filter_apply_flags(filter, flags);
    return filtergen_cprod(filter, &cb_ipfilter, &misc);
}
