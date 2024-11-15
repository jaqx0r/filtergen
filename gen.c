/* filter compilation routines
 *
 * Copyright (c) 2002 Matthew Kirkwood
 * Copyright (c) 2003,2004 Jamie Wilkinson <jaq@spacepants.org>
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

#include "gen.h"

#include <netinet/in.h>
#include <string.h>

#include "error.h"
#include "filter.h"
#include "flags.h"

void applydefaults(struct filterent *e, long flags) {
  if (!e->rtype) {
    if (flags & FF_LOCAL)
      e->rtype = LOCALONLY;
    else if (flags & FF_ROUTE)
      e->rtype = ROUTEDONLY;
  }
}

int checkmatch(const struct filterent *e) {
  int r = 0;
#define MUST(t)                                                                \
  do                                                                           \
    if (!(e->t)) {                                                             \
      filter_error(e->pos, "%s missing from filter", #t);                      \
      r++;                                                                     \
    }                                                                          \
  while (0)
  if (!e->subgroup)
    MUST(target);
  if (!e->groupname) {
    MUST(direction);
    MUST(iface);
  }
#undef MUST

  if ((e->u.ports.src.minstr || e->u.ports.dst.minstr) &&
      (e->proto.num != IPPROTO_TCP) && (e->proto.num != IPPROTO_UDP)) {
    filter_error(e->pos, "can only use ports with tcp or udp");
    r++;
  }

  if (e->u.icmp && (e->proto.num != IPPROTO_ICMP)) {
    filter_error(e->pos, "icmptype can only be used with icmp");
    r++;
  }

  if ((e->rtype == LOCALONLY) && (e->target == MASQ)) {
    filter_error(e->pos, "\"local\" and masquerading are incompatible");
    r++;
  }

  return r;
}

int __fg_apply(struct filterent *e, const struct filter *f, fg_callback *cb,
               struct fg_misc *misc);

int __fg_applylist(struct filterent *e, const struct filter *f, fg_callback *cb,
                   struct fg_misc *misc) {
  /* This is the interesting one.  The filters are
   * unrolled by now, so there's only one way to
   * follow it */
  int c = 0;
  for (; f; f = f->next) {
    int _c = __fg_apply(e, f, cb, misc);
    if (_c < 0)
      return _c;
    c += _c;
  }
  return c;
}

int __fg_applyone(struct filterent *e, const struct filter *f, fg_callback *cb,
                  struct fg_misc *misc) {
#define _NA(t, x, fmt)                                                         \
  if (x) {                                                                     \
    filter_error(f->pos, "filter has already defined a %s: \"" fmt "\"", t,    \
                 x);                                                           \
    return -1;                                                                 \
  }
#define NA(t, fmt) _NA(#t, e->t, fmt)

#define NC(type, str, fmt)                                                     \
  case F_##type:                                                               \
    _NA(str, ESET(e, type), fmt);                                              \
    e->whats_set |= (1 << F_##type);

  e->pos = f->pos;

  switch (f->type) {
    NC(TARGET, "target", "%d")
    e->target = f->u.target;
    break;

    NC(SUBGROUP, "subgroup", "%s") {
      struct filterent fe;
      int r;

      if (e->subgroup) {
        filter_error(e->pos, "cannot compose subgroups");
        return -1;
      }
      if (!cb->group) {
        filter_error(
            e->pos,
            "backend doesn't support grouping, but hasn't removed groups");
        return -1;
      }
      e->target = f->type;
      e->subgroup = f->u.sub.name;

      memset(&fe, 0, sizeof(fe));
      fe.groupname = f->u.sub.name;
      /* direction is special -- we need to save it */
      fe.direction = e->direction;
      cb->group(fe.groupname);
      if ((r = __fg_applylist(&fe, f->u.sub.list, cb, misc)) < 0)
        return r;

      break;
    }

    NC(DIRECTION, "direction and interface", "%d")
    NA(iface, "%s");
    e->direction = f->u.ifinfo.direction;
    e->iface = f->u.ifinfo.iface;
    break;

  case F_LOG:
    e->whats_set |= (1 << F_LOG);
    e->logmsg = f->u.logmsg;
    break;

#define _DV(tag, str, test, targ, source)                                      \
  case F_##tag:                                                                \
    _NA(str, e->test, "%s");                                                   \
    e->targ = f->u.source;                                                     \
    break
#define DV(tag, targ, source) _DV(tag, #targ, targ, targ, source)

    _DV(PROTO, "protocol", proto.name, proto, proto);
    _DV(SOURCE, "source address", srcaddr.addrstr, srcaddr, addrs);
    _DV(DEST, "destination address", dstaddr.addrstr, dstaddr, addrs);
    _DV(SPORT, "source port", u.ports.src.minstr, u.ports.src, ports);
    _DV(DPORT, "destination port", u.ports.src.maxstr, u.ports.dst, ports);
    DV(ICMPTYPE, u.icmp, icmp);
    DV(RTYPE, rtype, rtype);

  case F_ONEWAY:
    e->oneway = 1;
    break;

  case F_SIBLIST:
    return __fg_applylist(e, f->u.sib, cb, misc);

  default:
    filter_error(e->pos, "invalid filter type %d", f->type);
    return -1;
  }

  if (f->negate)
    e->whats_negated |= (1 << f->type);

  return 0;
}

int __fg_apply(struct filterent *_e, const struct filter *f, fg_callback *cb,
               struct fg_misc *misc) {
  struct filterent e = *_e;

  /* Looks like we're all done */
  if (!f) {
    applydefaults(&e, misc->flags);
    if (checkmatch(&e)) {
      filter_error(e.pos, "filter definition incomplete");
      return -1;
    }
    return cb->rule(&e, misc);
  }

  int r = __fg_applyone(&e, f, cb, misc);
  if (r) {
    return r;
  }
  return __fg_apply(&e, f->child, cb, misc);
}

int filtergen_cprod(struct filter *filter, fg_callback *cb,
                    struct fg_misc *misc) {
  struct filterent e;
  memset(&e, 0, sizeof(e));
  return __fg_applylist(&e, filter, cb, misc);
}
