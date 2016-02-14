/* filter generator, iptables driver
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
#define REJECT 0x01
#define A_TCP 0x10
#define A_UDP 0x20

/* full path to ip{,6}tables executables */
#define IPTABLES "/sbin/iptables"
#define IP6TABLES "/sbin/ip6tables"

static char *appip(char *r, const struct addr_spec *h) {
  APPS(r, h->addrstr);
  if (h->maskstr)
    APP2(r, "/", h->maskstr);
  return r;
}
#define APPIP(r, h) (r = appip(r, h))
#define APPIP2(f, r, h) (APPS(r, f), APPIP(r, h))

static char *appport(char *r, const struct port_spec *h) {
  APPS(r, h->minstr);
  if (h->maxstr)
    APP2(r, ":", h->maxstr);
  return r;
}
#define APPPORT(r, h) (r = appport(r, h))
#define APPPORT2(f, r, h) (APPS(r, f), APPPORT(r, h))

static int cb_iptables_rule_common(const struct filterent *ent,
                                   struct fg_misc *misc, sa_family_t family,
                                   const char *iptables) {
  char *rulechain = NULL, *revchain = NULL, *natchain = NULL;
  char *ruletarget = NULL, *revtarget = NULL, *nattarget = NULL;
  char *natrule = NULL, *rule = NULL, *rule_r = NULL;
  char *forchain = NULL, *forrevchain = NULL;
  char *fortarget = NULL, *forrevtarget = NULL;
  char *subchain = NULL, *subtarget = NULL;
  int neednat = 0, needret = 0;
  int islocal = (ent->rtype != ROUTEDONLY);
  int isforward = (ent->rtype != LOCALONLY);
  long *feat = (long *)misc->misc;
  enum filtertype target = ent->target;
  int orules = 0;

  /* nat rule? */
  if ((target == MASQ) || (target == REDIRECT)) {
    neednat = 1;
    if (family == AF_INET6) {
      fprintf(stderr, "can't NAT with IPv6\n");
      return -1;
    }
    if ((target == MASQ) && (ent->direction == INPUT)) {
      fprintf(stderr, "can't masquerade on input\n");
      return -1;
    } else if ((target == REDIRECT) && (ent->direction == OUTPUT)) {
      fprintf(stderr, "can't redirect on output\n");
      return -1;
    }
  }

  /* sub-stuff? */
  if (target == F_SUBGROUP) {
    subtarget = strapp(strdup(ent->subgroup), "-");
    needret = 1;
  } else
    subtarget = strdup("");

  if (ent->groupname)
    subchain = strapp(strdup(ent->groupname), "-");
  else
    subchain = strdup("");

  switch (ent->direction) {
  case INPUT:
    natchain = strdup("PREROUTING");
    rulechain = strdup("INPUT");
    revchain = strdup("OUTPUT");
    forchain = strdup("FORWARD");
    forrevchain = strdup("FORW_OUT");
    if (ent->iface && strcmp(ent->iface, "*")) {
      if (NEG(DIRECTION)) {
        APPS(natrule, "!");
        APPS(rule, "!");
        APPS(rule_r, "!");
      }
      if (strcmp(ent->iface, "*")) {
        APPSS2(natrule, "-i", ent->iface);
        APPSS2(rule, "-i", ent->iface);
        APPSS2(rule_r, "-o", ent->iface);
      }
    }
    break;
  case OUTPUT:
    natchain = strdup("POSTROUTING");
    rulechain = strdup("OUTPUT");
    revchain = strdup("INPUT");
    forchain = strdup("FORW_OUT");
    forrevchain = strdup("FORWARD");
    if (ent->iface && strcmp(ent->iface, "*")) {
      if (NEG(DIRECTION)) {
        APPS(natrule, "!");
        APPS(rule, "!");
        APPS(rule_r, "!");
      }
      if (strcmp(ent->iface, "*")) {
        APPSS2(natrule, "-o", ent->iface);
        APPSS2(rule, "-o", ent->iface);
        APPSS2(rule_r, "-i", ent->iface);
      }
    }
    break;
  default:
    fprintf(stderr, "unknown direction: %d\n", ent->direction);
    abort();
  }

  /* state and reverse rules here */
  /* FIXME: state established on reverse for every rule, not just
   * specifically udp and tcp */
  if (ent->proto.name) {
    int needstate = 0;

    APPSS2(natrule, "-p", ent->proto.name);
    APPSS2(rule, "-p", ent->proto.name);
    APPSS2(rule_r, "-p", ent->proto.name);
    switch (ent->proto.num) {
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
    if (needstate) {
      APPS(rule, "-m state --state NEW,ESTABLISHED");
      APPS(rule_r, "-m state --state ESTABLISHED");
    }
  }

  if (ent->srcaddr.addrstr) {
    NEGA(natrule, SOURCE);
    NEGA(rule, SOURCE);
    NEGA(rule_r, SOURCE);
    APPIP2("-s", natrule, &ent->srcaddr);
    APPIP2("-s", rule, &ent->srcaddr);
    APPIP2("-d", rule_r, &ent->srcaddr);
  }
  if (ent->dstaddr.addrstr) {
    NEGA(natrule, DEST);
    NEGA(rule, DEST);
    NEGA(rule_r, DEST);
    APPIP2("-d", natrule, &ent->dstaddr);
    APPIP2("-d", rule, &ent->dstaddr);
    APPIP2("-s", rule_r, &ent->dstaddr);
  }

  switch (ent->proto.num) {
  case 0:
    break;
  case IPPROTO_UDP:
  case IPPROTO_TCP:
    if (ent->u.ports.src.minstr) {
      NEGA(natrule, SPORT);
      NEGA(rule, SPORT);
      NEGA(rule_r, SPORT);
      APPPORT2("--sport", natrule, &ent->u.ports.src);
      APPPORT2("--sport", rule, &ent->u.ports.src);
      APPPORT2("--dport", rule_r, &ent->u.ports.src);
    }
    if (ent->u.ports.dst.minstr) {
      NEGA(natrule, DPORT);
      NEGA(rule, DPORT);
      NEGA(rule_r, DPORT);
      APPPORT2("--dport", natrule, &ent->u.ports.dst);
      APPPORT2("--dport", rule, &ent->u.ports.dst);
      APPPORT2("--sport", rule_r, &ent->u.ports.dst);
    }
    break;
  case IPPROTO_ICMP:
    if (ent->u.icmp) {
      NEGA(natrule, ICMPTYPE);
      APPSS2(natrule, "--icmp-type", ent->u.icmp);
      NEGA(rule, ICMPTYPE);
      APPSS2(rule, "--icmp-type", ent->u.icmp);
    }
    break;
  default:
    ;
  }

  APPS(natrule, "-j");
  APPS(rule, "-j");
  APPS(rule_r, "-j");

  /* The "rule+1" in the printfs below are an ugly hack to
   * prevent a double-space in the output rule */

  /* Yuck, separate rules for logging packets.  Be still my
   * beating lunch.
   *
   * Logging and target rules have to be the last bits
   * before output, or this doesn't work.  This will also
   * fail if any mangling has been done above.
   */
  if (ESET(ent, LOG)) {
    char *lc, *la, *ls;
    if (ent->logmsg) {
      lc = strdup(" --log-prefix=");
      la = ent->logmsg;
      ls = strdup("\" \"");
    } else
      lc = la = ls = strdup("");
    if (islocal)
      orules++, oprintf("%s -A %s %s LOG%s%s%s\n", iptables, rulechain,
                        rule + 1, lc, la, ls);
    if (isforward)
      orules++, oprintf("%s -A %s %s LOG%s%s%s\n", iptables, forchain, rule + 1,
                        lc, la, ls);
  }

  /* Do this twice, once for NAT, once for filter */
  if (neednat) {
    switch (target) {
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

  switch (target) {
  case MASQ:
  case REDIRECT:
  case T_ACCEPT:
    ruletarget = revtarget = fortarget = forrevtarget = strdup("ACCEPT");
    switch (ent->direction) {
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
    needret = 0;
    break;
  case T_REJECT:
    ruletarget = fortarget = strdup("REJECT");
    needret = 0;
    *feat |= T_REJECT;
    break;
  case F_SUBGROUP:
    switch (ent->direction) {
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

  if ((misc->flags & FF_LSTATE) && (target != T_REJECT))
    needret = 0;

  if (ent->oneway)
    needret = 0;

  if (neednat)
    orules++, oprintf("%s -t nat -A %s%s %s %s%s\n", iptables, subchain,
                      natchain, natrule + 1, subtarget, nattarget);
  if (islocal)
    orules++, oprintf("%s -A %s%s %s %s%s\n", iptables, subchain, rulechain,
                      rule + 1, subtarget, ruletarget);
  if (needret)
    orules++, oprintf("%s -I %s%s %s %s%s\n", iptables, subchain, revchain,
                      rule_r + 1, subtarget, revtarget);
  if (isforward) {
    orules++, oprintf("%s -A %s%s %s %s%s\n", iptables, subchain, forchain,
                      rule + 1, subtarget, fortarget);
    if (needret)
      orules++, oprintf("%s -I %s%s %s %s%s\n", iptables, subchain, forrevchain,
                        rule_r + 1, subtarget, forrevtarget);
  }

  free(natrule);
  free(rule);
  free(rule_r);
  free(subchain);
  free(subtarget);
  return orules;
}

static int cb_iptables_rule(const struct filterent *ent, struct fg_misc *misc) {
  return cb_iptables_rule_common(ent, misc, AF_INET, IPTABLES);
}

static int cb_ip6tables_rule(const struct filterent *ent,
                             struct fg_misc *misc) {
  return cb_iptables_rule_common(ent, misc, AF_INET6, IP6TABLES);
}

static int cb_iptables_group_common(const char *name, const char *iptables) {
  oprintf("for f in INPUT OUTPUT FORWARD FORW_OUT; do %s -N %s-$f; done\n",
          iptables, name);
  return 4;
}

static int cb_iptables_group(const char *name) {
  return cb_iptables_group_common(name, IPTABLES);
}

static int cb_ip6tables_group(const char *name) {
  return cb_iptables_group_common(name, IP6TABLES);
}

static int fg_iptables_common(struct filter *filter, int flags,
                              sa_family_t family, const char *iptables) {
  long feat = 0;
  int r = 0;
  struct fg_misc misc = {flags, &feat};
  fg_callback cb_iptables = {
      .rule = family == AF_INET ? cb_iptables_rule : cb_ip6tables_rule,
      .group = family == AF_INET ? cb_iptables_group : cb_ip6tables_group,
  };
  const int nchains = 3;

  filter_unroll(&filter);
  filter_apply_flags(filter, flags);

  if (!(flags & FF_NOSKEL)) {
    oputs("CHAINS=\"INPUT OUTPUT FORWARD\"");
    oputs("");

    oputs("# Flush/remove rules, set policy");
    oprintf("for f in $CHAINS; do %s -P $f DROP; done\n", iptables);
    oprintf("%s -F; %s -X\n", iptables, iptables);
    if (family == AF_INET)
      oprintf("%s -t nat -F; %s -t nat -X\n", iptables, iptables);
    oputs("");

    oputs("# Create FORW_OUT chain");
    oprintf("%s -N FORW_OUT\n", iptables);
    oputs("");

    oputs("# Setup INVALID chain");
    oprintf("%s -N INVALID\n", iptables);
#if 0
	oprintf("%s -A INVALID -j LOG --log-prefix \"invalid \"\n", iptables);
#endif
    oprintf("%s -A INVALID -j DROP\n", iptables);
    oprintf("for f in $CHAINS; do\n"
            "\t%s -I $f -m state --state INVALID -j INVALID;\n"
            "done\n",
            iptables);
    oputs("");
    r += nchains;
  }
  if ((r = filtergen_cprod(filter, &cb_iptables, &misc)) < 0)
    return r;
  if (!(flags & FF_NOSKEL)) {
    if ((flags & FF_LSTATE) && (feat & (A_TCP | A_UDP))) {
      oputs("for f in $CHAINS; do");
      if (feat & A_TCP) {
        r += nchains;
        oprintf("\t%s -I $f -p tcp ! --syn -m state --state ESTABLISHED -j "
                "ACCEPT;\n",
                iptables);
      }
      if (feat & A_UDP) {
        r += nchains;
        oprintf("\t%s -I $f -p udp -m state --state ESTABLISHED -j ACCEPT;\n",
                iptables);
      }
      oputs("done");
    }
#if 0
	oprintf("for f in $CHAINS; do %s -A $f -j LOG; done\n", iptables);
	r += nchains;
#endif
  }
  return r;
}

int fg_iptables(struct filter *filter, int flags) {
  return fg_iptables_common(filter, flags, AF_INET, IPTABLES);
}

int fg_ip6tables(struct filter *filter, int flags) {
  return fg_iptables_common(filter, flags, AF_INET6, IP6TABLES);
}

/* Rules which just flush the packet filter */
static int flush_iptables_common(enum filtertype policy, sa_family_t family,
                                 const char *iptables) {
  char *ostr;

  oputs("CHAINS=\"INPUT OUTPUT FORWARD\"");
  oputs("");

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
  oprintf("for f in $CHAINS; do %s -P $f %s; done\n", iptables, ostr);
  oprintf("%s -F; %s -X\n", iptables, iptables);
  if (family == AF_INET)
    oprintf("%s -t nat -F; %s -t nat -X\n", iptables, iptables);

  return 0;
}

int flush_iptables(enum filtertype policy) {
  return flush_iptables_common(policy, AF_INET, IPTABLES);
}

int flush_ip6tables(enum filtertype policy) {
  return flush_iptables_common(policy, AF_INET, IP6TABLES);
}
