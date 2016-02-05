/* filtergen emitter for filtergen language
 *
 * Copyright (c) 2005 Jamie Wilkinson
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "util.h"

/** Callback for emitting filterents containing rules. */
static int out_filtergen_rule(const struct filterent *ent,
                              struct fg_misc *misc __attribute__((unused))) {

  char *rule = NULL;

  switch (ent->direction) {
  case INPUT:
    APPSS2(rule, "input", ent->iface);
    break;
  case OUTPUT:
    APPSS2(rule, "output", ent->iface);
    break;
  default:
    break;
  }

  if (ent->srcaddr.addrstr) {
    APPSS2(rule, "source", ent->srcaddr.addrstr);
    if (ent->srcaddr.maskstr)
      APP2(rule, "/", ent->srcaddr.maskstr);
  }

  if (ent->dstaddr.addrstr) {
    APPSS2(rule, "dest", ent->dstaddr.addrstr);
    if (ent->dstaddr.maskstr)
      APP2(rule, "/", ent->dstaddr.maskstr);
  }

  if (ent->proto.name)
    APPSS2(rule, "proto", ent->proto.name);

  if (ent->u.ports.src.minstr) {
    APPSS2(rule, "sport", ent->u.ports.src.minstr);
    if (ent->u.ports.src.maxstr)
      APP2(rule, ":", ent->u.ports.src.maxstr);
  }

  if (ent->u.ports.dst.minstr) {
    APPSS2(rule, "dport", ent->u.ports.dst.minstr);
    if (ent->u.ports.dst.maxstr)
      APP2(rule, ":", ent->u.ports.dst.maxstr);
  }

  if (ent->u.icmp)
    APPSS2(rule, "icmptype", ent->u.icmp);

  if (ESET(ent, LOG))
    APPS(rule, "log");

  switch (ent->target) {
  case T_ACCEPT:
    APPS(rule, "accept");
    break;
  case DROP:
    APPS(rule, "drop");
    break;
  case T_REJECT:
    APPS(rule, "reject");
    break;
  default:
    break;
  }

  APPS(rule, ";");
  oputs(rule);
  free(rule);
  return 1;
}

/** Emit the filter's internal representation into the filtergen language */
int emit_filtergen(struct filter *filter, int flags) {
  struct fg_misc misc = {flags, NULL};
  fg_callback out_filtergen_cb = {rule : out_filtergen_rule, NULL};

  filter_nogroup(filter);
  filter_unroll(&filter);
  filter_apply_flags(filter, flags);
  return filtergen_cprod(filter, &out_filtergen_cb, &misc);
}
