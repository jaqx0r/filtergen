/* data structure building routines
 *
 * Copyright (c) 2002,2003 Matthew Kirkwood
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

#include "filter.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input/sourcepos.h"

static struct filter *__new_filter(enum filtertype type,
                                   struct sourceposition *pos) {
  struct filter *f;
  if ((f = calloc(1, sizeof(*f)))) {
    f->type = type;
    f->pos = pos;
  }
  return f;
}

struct filter *new_filter_oneway(struct sourceposition *pos) {
  return __new_filter(F_ONEWAY, pos);
}

struct filter *new_filter_target(enum filtertype target,
                                 struct sourceposition *pos) {
  struct filter *f;
  if ((f = __new_filter(F_TARGET, pos))) {
    f->u.target = target;
  }
  return f;
}

struct filter *new_filter_log(enum filtertype type, const char *text,
                              struct sourceposition *pos) {
  struct filter *f;
  if ((f = __new_filter(type, pos))) {
    f->u.logmsg = text ? strdup(text) : NULL;
  }
  return f;
}

struct filter *new_filter_rtype(enum filtertype rtype,
                                struct sourceposition *pos) {
  struct filter *f;
  if ((f = __new_filter(F_RTYPE, pos))) {
    f->u.rtype = rtype;
  }
  return f;
}

struct filter *new_filter_neg(struct filter *sub) {
  struct filter *f;
  if ((f = __new_filter(F_NEG, sub->pos))) {
    f->u.neg = sub;
  }
  return f;
}

struct filter *new_filter_sibs(struct filter *list) {
  struct filter *f;
  if (!list)
    return NULL;
  if ((f = __new_filter(F_SIBLIST, list->pos))) {
    f->u.sib = list;
  }
  return f;
}

struct filter *new_filter_subgroup(char *name, struct filter *list) {
  struct filter *f;
  if ((f = __new_filter(F_SUBGROUP, list->pos))) {
    f->u.sub.name = name;
    f->u.sub.list = list;
  }
  return f;
}

struct filter *new_filter_proto(enum filtertype type, const char *name,
                                struct sourceposition *pos) {
  struct filter *f;
  struct protoent *e;
  int pn;

  if (!str_to_int(name, &pn))
    e = getprotobynumber(pn);
  else
    e = getprotobyname(name);

  if (!e) {
    filter_error(pos, "don't know protocol \"%s\"", name);
    return NULL;
  }

  if ((f = __new_filter(type, pos))) {
    f->u.proto.num = e->p_proto;
    f->u.proto.name = strdup(e->p_name);
  }
  return f;
}

struct filter *new_filter_device(enum filtertype type, const char *iface,
                                 struct sourceposition *pos) {
  struct filter *f;
  if ((f = __new_filter(F_DIRECTION, pos))) {
    f->u.ifinfo.direction = type;
    f->u.ifinfo.iface = strdup(iface);
  }
  return f;
}

struct filter *new_filter_host(enum filtertype type, const char *matchstr,
                               sa_family_t family, struct sourceposition *pos) {
  struct filter *f;
  char *mask;
  int i;

  if (!(f = __new_filter(type, pos)))
    return f;

  f->u.addrs.family = family;
  f->u.addrs.addrstr = strdup(matchstr);
  if ((mask = strchr(f->u.addrs.addrstr, '/'))) {
    *mask++ = 0;
    switch (family) {
    case AF_INET:
      if (!str_to_int(mask, &i)) {
        /* Netmask like foo/24 */
        uint32_t l = 0xffffffff;
        if (i < 0 || i > 32) {
          filter_error(pos, "can't parse netmask \"%s\"", mask);
          return NULL;
        }
        if (!i)
          l = 0;
        else {
          i = 32 - i;
          l >>= i;
          l <<= i;
        }
        f->u.addrs.u.inet.mask.s_addr = htonl(l);
      } else {
        /* Better be a /255.255.255.0 mask */
        if (!inet_aton(mask, &f->u.addrs.u.inet.mask)) {
          filter_error(pos, "can't parse netmask \"%s\"", mask);
          return NULL;
        }
      }
      f->u.addrs.maskstr = strdup(inet_ntoa(f->u.addrs.u.inet.mask));
      break;
    case AF_INET6:
      if (!str_to_int(mask, &i)) {
        /* Netmask like foo/128 */
        unsigned char l[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        unsigned char *p = l + 15;
        int o;
        if (i < 0 || i > 128) {
          filter_error(pos, "can't parse netmask \"%s\"", mask);
          return NULL;
        }
        if (i != 0) {
          o = 128 - i;
          while (o > 8) {
            *p = 0x00;
            o -= 8;
          }
          if (!i)
            *p = 0x00;
          else {
            *p >>= o;
            *p <<= o;
          }
        }
        memcpy(f->u.addrs.u.inet6.mask.s6_addr, l, sizeof l);
        f->u.addrs.maskstr = int_to_str_dup(i);
      } else {
        filter_error(pos, "can't parse netmask \"%s\"", mask);
        return NULL;
      }
      break;
    default:
      filter_error(pos,
                   "can't parse netmask \"%s\" for invalid address family %d",
                   mask, family);
      return NULL;
    }
  }

  return f;
}

struct filter *new_filter_ports(enum filtertype type, const char *matchstr,
                                struct sourceposition *pos) {
  struct filter *f;
  struct servent *s;
  char *min, *max;
  int imin, imax;

  min = strdup(matchstr);
  if ((max = strchr(min, ':'))) {
    *max++ = 0;
    max = strdup(max);
  }

  if (str_to_int(min, &imin)) {
    if (!(s = getservbyname(min, NULL)))
      imin = -1;
    else {
      free(min);
      min = strdup(s->s_name);
      imin = ntohs(s->s_port);
    }
  }
  if (max) {
    if (str_to_int(max, &imax)) {
      if (!(s = getservbyname(max, NULL)))
        imax = -1;
      else {
        free(max);
        max = strdup(s->s_name);
        imax = ntohs(s->s_port);
      }
    }
  } else
    imax = imin;

  if ((f = __new_filter(type, pos))) {
    f->u.ports.min = imin;
    f->u.ports.max = imax;
    f->u.ports.minstr = min;
    f->u.ports.maxstr = max;
  }
  return f;
}

struct filter *new_filter_icmp(enum filtertype type, const char *matchstr,
                               struct sourceposition *pos) {
  struct filter *f;
  if ((f = __new_filter(type, pos))) {
    f->u.icmp = strdup(matchstr);
  }
  return f;
}

/*
 * These functions DAGify the parsed filter structure.
 * This means that we can walk all paths down the DAG
 * merely by following ->child and ->next.
 */

static void filter_append(struct filter *f, struct filter *x) {
  if (!f) {
    fprintf(stderr, "filter not defined");
    return;
  }
  if (!x)
    return;

  /* We have to be paranoid about making loops here */
  while ((f->type != F_SIBLIST) && f->child) {
    if (f == x)
      return;
    f = f->child;
  }
  if (f == x)
    return;

  if (f->type == F_SIBLIST) {
    if (f->child) {
      filter_error(
          f->pos,
          "corrupt siblist contains child node, __filter_unroll not called?");
      abort();
    }
    for (f = f->u.sib; f; f = f->next)
      filter_append(f, x);
  } else
    f->child = x;
}

/*
 * The easy bit of a cross-product.  Basically we ensure that no
 * filter node has more than one path out.
 * 1. We push sibling->child down to the end of the component
 *    sub-lists, and
 * 2. Ensure that negated entries have only a single component
 *    hanging off them.
 */
void __filter_unroll(struct filter *f) {
  struct filter *c, *s;

  if (!f)
    return;

  /* depth first */
  __filter_unroll(c = f->child);

  /* check this node */
  switch (f->type) {
  case F_SIBLIST:
    for (s = f->u.sib; s; s = s->next) {
      __filter_unroll(s);
      filter_append(s, c);
    }
    f->child = NULL;
    break;
  case F_SUBGROUP:
    __filter_unroll(f->u.sub.list);
    break;
  case F_NEG:
    abort();
  default:
    break;
  }

  /* lastly, go sideways */
  __filter_unroll(f->next);
}

void __filter_neg_expand(struct filter **f, int neg) {
  if (!*f)
    return;
  __filter_neg_expand(&(*f)->child, neg);
  __filter_neg_expand(&(*f)->next, neg);

  switch ((*f)->type) {
  case F_SIBLIST:
    if (neg && (*f)->u.sib && (*f)->u.sib->next) {
      filter_error((*f)->pos, "can't negate conjunctions");
      exit(1);
    }
    __filter_neg_expand(&(*f)->u.sib, neg);
    break;
  case F_SUBGROUP:
    if (neg) {
      filter_error((*f)->pos, "can't negate subgroups");
      exit(1);
    }
    __filter_neg_expand(&(*f)->u.sub.list, neg);
    break;
  case F_NEG: {
    struct filter *c = (*f)->child, *n = (*f)->next;
    *f = (*f)->u.neg;
    neg = !neg;
    __filter_neg_expand(f, neg);
    if (c)
      filter_append(*f, c);
    (*f)->next = n;
    break;
  }
  default:
    break;
  }
  (*f)->negate = neg;
}

/* Move targets to end of each list */
void __filter_targets_to_end(struct filter **f) {
  if (!*f)
    return;
  if (((*f)->type == F_TARGET) && (*f)->child) {
    struct filter *c = (*f)->child;

    /* Unlink this one */
    (*f)->child = NULL;

    /* Append ourselves */
    filter_append(c, (*f));

    /* Tell them upstairs */
    *f = c;
  } else {
    __filter_targets_to_end(&(*f)->child);
    __filter_targets_to_end(&(*f)->next);
  }
}

void filter_unroll(struct filter **f) {
  __filter_neg_expand(f, 0);
  __filter_unroll(*f);
  __filter_targets_to_end(f);
}

void filter_nogroup(struct filter *f) {
  if (!f)
    return;
  switch (f->type) {
  case F_SUBGROUP:
    f->u.sib = f->u.sub.list;
    f->type = F_SIBLIST;
  /* fall through */
  case F_SIBLIST:
    filter_nogroup(f->u.sib);
    break;
  default:
    ;
  }
  filter_nogroup(f->child);
  filter_nogroup(f->next);
}

/* Remove negations by reordering tree:
 *	NEG(ent)->child,next
 * becomes
 *	ent->next,child,next
 */
void filter_noneg(struct filter **f) { if (f) { };
  return;
}

void __attribute__((__format__(__printf__, 2, 3)))
filter_error(struct sourceposition *pos, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  if (pos && pos->first_line) {
    fprintf(stderr, "%s:%d.%d-%d.%d: error: ", pos->filename, pos->first_line,
            pos->first_column, pos->last_line, pos->last_column);
  }
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}
