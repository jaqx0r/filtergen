/* filter generator, Cisco IOS driver
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

/* XXX - does this need skeleton rules? */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

static char *appip(char *r, const struct addr_spec *h)
{
    if(!h->addrstr) return APPS(r, "any");
    if(!h->maskstr) return APPS2(r, "host ", h->addrstr);
    return APPSS2(r, h->addrstr, h->maskstr);
}

static char *appport(char *r, const struct port_spec *p, int neg)
{
    if(!p->minstr) return APPS(r, "any");

    if(!p->maxstr)
	return APPS2(r, neg ? "ne " : "eq ", p->minstr);

    if(neg) abort();
    APPS(r, "range");
    APPSS2(r, p->minstr, p->maxstr);
    return r;
}

static int cb_cisco_rule(const struct filterent *ent, struct fg_misc *misc __attribute__((unused)))
{
    char *rule = NULL, *rule_r = NULL;
    int needret = 0, needports = 1;

    APP(rule, "access-list ");
    APP(rule_r, "access-list ");

    /* access list name */
    if(ent->iface) {
	APP2(rule, ent->iface, "-");
	APP2(rule_r, ent->iface, "-");
    }
    switch(ent->direction) {
      case INPUT:	APP(rule, "IN"); APP(rule_r, "OUT"); break;
      case OUTPUT:	APP(rule, "OUT"); APP(rule_r, "IN"); break;
      default: fprintf(stderr, "unknown direction\n"); abort();
    }

    /* target */
    switch(ent->target) {
      case T_ACCEPT:	APPS(rule, "permit"); APPS(rule_r, "permit"); break;
      case DROP: 	APPS(rule, "deny"); APPS(rule_r, "deny"); break;
      case T_REJECT:
	fprintf(stderr, "warning: Cisco IOS does not support REJECT\n");
	APPS(rule, "deny");
	APPS(rule_r, "deny");
	break;
      default: abort();
    }

    /* protocol */
    if(ent->proto.name) {
	APPS(rule, ent->proto.name);
	APPS(rule_r, ent->proto.name);
	switch(ent->proto.num) {
	  case IPPROTO_TCP: case IPPROTO_UDP:
	    needret++;
	    break;
	  default:
	    needports = 0;
	}
    } else {
	APPS(rule, "ip"); APPS(rule_r, "ip");
    }

    rule = appip(rule, &ent->srcaddr);
    rule = appip(rule, &ent->dstaddr);
    rule_r = appip(rule_r, &ent->dstaddr);
    rule_r = appip(rule_r, &ent->srcaddr);

    if(needports) {
	rule = appport(rule, &ent->u.ports.src, NEG(SPORT));
	rule = appport(rule, &ent->u.ports.dst, NEG(DPORT));
	rule_r = appport(rule_r, &ent->u.ports.dst, NEG(DPORT));
	rule_r = appport(rule_r, &ent->u.ports.src, NEG(SPORT));
    }

    if(ent->proto.num == IPPROTO_TCP) APPS(rule_r, "established");
    if(ESET(ent,LOG)) APPS(rule, "log");

    if(ent->oneway) needret = 0;

    oputs(rule);
    if(needret) oputs(rule_r);

    free(rule); free(rule_r);
    return 1 + !!needret;
}

int fg_cisco(struct filter *filter, int flags)
{
    struct fg_misc misc = { flags, NULL };
    fg_callback cb_cisco = {
	rule:	cb_cisco_rule, NULL
    };
    oputs("# Warning: This backend is not complete and "
	  "can generate broken rulesets.");
    filter_nogroup(filter);
    filter_unroll(&filter);
    filter_apply_flags(filter, flags);
    return filtergen_cprod(filter, &cb_cisco, &misc);
}
