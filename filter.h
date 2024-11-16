/* types and functions used throughout filtergen
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

#ifndef _FK_FILTER_H
#define _FK_FILTER_H

#include <netinet/in.h>
#include <sys/un.h>

#include "input/sourcepos.h"

/**
 * If it doesn't begin with "F_", it's a simple token, not an
 * filter type
 */
enum filtertype {
  YYEOF = 0,
  F_DIRECTION,
  F_TARGET,
  F_SOURCE,
  F_DEST,
  F_SPORT,
  F_DPORT,
  F_ICMPTYPE,
  F_PROTO,
  F_NEG,
  F_SIBLINGLIST,
  F_SUBGROUP,
  F_LOG,
  F_ROUTINGTYPE,
  F_ONEWAY,
  /* this must be the last real filter type */
  F_FILTER_MAX,
  /* parser use only */
  INPUT,
  OUTPUT,
  /* lex uses "ACCEPT" and "REJECT" */
  T_ACCEPT,
  DROP,
  T_REJECT,
  MASQ,
  REDIRECT,
  OPENBRACE,
  CLOSEBRACE,
  OPENBRACKET,
  CLOSEBRACKET,
  SEMICOLON,
  STRING,
  INCLUDE,
  LOCALONLY,
  ROUTEDONLY, /* for F_RTYPE */
  TEXT,       /* for F_LOG */
};

/** Structures which appear in both the parse tree and the output rule */
struct proto_spec {
  int num;
  char *name;
};

struct addr_spec {
  sa_family_t family;
  union {
    struct {
      struct in_addr addr, mask;
    } inet;
    struct {
      struct in6_addr addr, mask;
    } inet6;
  } u;
  char *addrstr, *maskstr;
};

struct port_spec {
  int min, max;
  char *minstr, *maxstr;
};

/** This is basically just a parse tree */
struct filter {
  enum filtertype type;
  union {
    struct {
      enum filtertype direction;
      char *iface;
    } ifinfo;
    enum filtertype target;
    char *logmsg;
    enum filtertype rtype;
    struct addr_spec addrs;
    struct port_spec ports;
    char *icmp;
    struct proto_spec proto;
    struct filter *neg;
    struct filter *siblings;
    struct {
      char *name;
      struct filter *list;
    } sub;
  } u;
  struct filter *child, *next;

  /* infernal use only */
  int negate;

  struct sourceposition *pos;
};

struct filtergen_opts {
  sa_family_t family;
};

typedef struct filter *filter_tctor(enum filtertype,
                                    struct sourceposition *pos);

filter_tctor new_filter_target, new_filter_rtype;

struct filter *new_filter_neg(struct filter *sub);
struct filter *new_filter_siblings(struct filter *list);
struct filter *new_filter_subgroup(char *name, struct filter *list);

typedef struct filter *filter_ctor(enum filtertype, const char *,
                                   struct sourceposition *pos);

filter_ctor new_filter_device, new_filter_ports, new_filter_icmp,
    new_filter_proto, new_filter_log;

struct filter *new_filter_host(enum filtertype, const char *, sa_family_t,
                               struct sourceposition *pos);
struct filter *new_filter_oneway(struct sourceposition *pos);

/* filter manipulations */
void filter_unroll(struct filter **f);
void filter_nogroup(struct filter *f);
void filter_noneg(struct filter **f);

/** Filter output drivers. */
typedef int filtergen(struct filter *filter, int flags);

filtergen fg_iptables, fg_ip6tables, fg_iptrestore, fg_ip6trestore, fg_ipchains,
    fg_ipfilter, fg_cisco, emit_filtergen;

/** Filter flush output drivers. */
typedef int filter_flush(enum filtertype policy);

filter_flush flush_iptables, flush_ip6tables, flush_iptrestore,
    flush_ip6trestore, flush_ipchains;

#endif /* _FK_FILTER_H */
