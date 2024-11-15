/* Filter generator methods and structures.
 *
 * Copyright (c) 2002 Matthew Kirkwood
 * Copyright (c) 2024 Jamie Wilkinson
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

#ifndef GEN_H
#define GEN_H

#include "filter.h"

#define ESET(e, f) (e->whats_set & (1 << F_##f))

struct filterent {
  /* Either direction+iface or groupname must be set */
  enum filtertype direction;
  char *iface;
  char *groupname;

  /* One of these must be set */
  enum filtertype target;
  char *subgroup;

  /* These may or may not be set */
  int whats_set : F_FILTER_MAX;
  int whats_negated : F_FILTER_MAX;
  struct addr_spec srcaddr, dstaddr;

  enum filtertype rtype;
  struct proto_spec proto;
  char *logmsg;
  int oneway;

  /* We need this not to be a union, for error-checking reasons */
  struct {
    struct {
      struct port_spec src, dst;
    } ports;
    char *icmp;
  } u;
  struct sourceposition *pos;
};

struct fg_misc {
  int flags;
  void *misc;
};
typedef int fg_cb_rule(const struct filterent *ent, struct fg_misc *misc);
typedef int fg_cb_group(const char *name);
typedef struct {
  fg_cb_rule *rule;
  fg_cb_group *group;
} fg_callback;
int filtergen_cprod(struct filter *filter, fg_callback *cb,
                    struct fg_misc *misc);

#endif /* GEN_H */
