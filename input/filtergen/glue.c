/* conversion glue between ast and filter structures
 *
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

#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "filter.h"
#include "flags.h"
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/resolver.h"
#include "input/filtergen/scanner.h"
#include "input/input.h"
#include "input/sourcefile.h"
#include "input/sourcepos.h"

int convtrace = 0;

#define eprint(x)                                                              \
  if (convtrace)                                                               \
  fprintf(stderr, x)

struct filter *convert_specifier_list(struct specifier_list_s *n,
                                      struct filtergen_opts *o);

struct filter *convert_subrule_list(struct subrule_list_s *n,
                                    struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting subrule_list\n");

  if (n->subrule_list) {
    res = convert_subrule_list(n->subrule_list, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
      if (n->specifier_list) {
        end->next = convert_specifier_list(n->specifier_list, o);
      }
    } else {
      filter_error(n->pos, "warning: convert_subrule_list returned NULL");
    }
  } else if (n->specifier_list) {
    res = convert_specifier_list(n->specifier_list, o);
  } else {
    filter_error(n->pos, "no content in subrule_list");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_compound_specifier(struct compound_specifier_s *r,
                                          struct filtergen_opts *o) {
  struct filter *res = NULL, *sub = NULL;

  eprint("converting compound_specifier\n");

  if (r->list) {
    sub = convert_subrule_list(r->list, o);
    if (sub) {
      res = new_filter_sibs(sub);
    }
  }
  if (res)
    res->pos = r->pos;
  free(r);
  return res;
}

struct filter *convert_direction_argument(struct direction_argument_s *n,
                                          int type) {
  struct filter *res = NULL;

  if (n->direction) {
    res = new_filter_device(type, n->direction, n->pos);
  } else {
    filter_error(n->pos, "no direction argument contents");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *
convert_direction_argument_list(struct direction_argument_list_s *n, int type,
                                struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting direction argument list\n");

  if (n->list) {
    res = convert_direction_argument_list(n->list, type, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
      if (n->arg) {
        end->next = convert_direction_argument(n->arg, type);
      }
    } else {
      filter_error(n->pos,
                   "warning: convert_direction_argument_list returned NULL\n");
    }
  } else {
    if (n->arg)
      res = convert_direction_argument(n->arg, type);
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_direction(struct direction_specifier_s *n,
                                 struct filtergen_opts *o) {
  struct filter *res = NULL;
  int type;

  eprint("converting direction specifier\n");

  switch (n->type) {
  case TOK_INPUT:
    type = INPUT;
    break;
  case TOK_OUTPUT:
    type = OUTPUT;
    break;
  default:
    filter_error(n->pos, "incorrect direction type encountered");
    type = YYEOF;
    break;
  }
  if (n->list) {
    res = new_filter_sibs(convert_direction_argument_list(n->list, type, o));
  } else {
    filter_error(n->pos, "no direction argument list");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_host_argument(struct host_argument_s *n, int type,
                                     struct filtergen_opts *o) {
  struct filter *res = NULL;
  char *h;

  eprint("converting host_argument\n");

  if (n->host) {
    if (n->mask) {
      if (asprintf(&h, "%s/%s", n->host, n->mask) < 0) {
        filter_error(n->pos,
                     "error: asprintf allocation failed when converting host "
                     "argument %s/%s",
                     n->host, n->mask);
      }
    } else {
      h = n->host;
    }
    res = new_filter_host(type, h, o->family, n->pos);
  } else {
    filter_error(n->pos, "no host part");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_host_argument_list(struct host_argument_list_s *n,
                                          int type, struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting host argument list\n");

  if (n->list) {
    res = convert_host_argument_list(n->list, type, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
      if (n->arg) {
        end->next = convert_host_argument(n->arg, type, o);
      }
    } else {
      filter_error(n->pos,
                   "warning: convert_host_argument_list returned NULL\n");
    }
  } else {
    if (n->arg)
      res = convert_host_argument(n->arg, type, o);
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_host_specifier(struct host_specifier_s *n,
                                      struct filtergen_opts *o) {
  struct filter *res = NULL;
  enum filtertype type;

  eprint("converting host specifier\n");

  switch (n->type) {
  case TOK_SOURCE:
    type = F_SOURCE;
    break;
  case TOK_DEST:
    type = F_DEST;
    break;
  default:
    filter_error(n->pos, "incorrect host type encountered");
    type = YYEOF;
    break;
  }
  if (n->list) {
    res = new_filter_sibs(convert_host_argument_list(n->list, type, o));
  } else {
    filter_error(n->pos, "no host argument list");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_protocol_argument(struct protocol_argument_s *n) {
  struct filter *res = NULL;

  eprint("converting protocol argument\n");

  if (n->proto) {
    res = new_filter_proto(F_PROTO, n->proto, n->pos);
  } else {
    filter_error(n->pos, "no protocol argument contents");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *
convert_protocol_argument_list(struct protocol_argument_list_s *n,
                               struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting protocol argument list\n");

  if (n->list) {
    res = convert_protocol_argument_list(n->list, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
      if (n->arg) {
        end->next = convert_protocol_argument(n->arg);
      }
    } else {
      filter_error(n->pos,
                   "warning: convert_protocol_argument_list returned NULL\n");
    }
  } else {
    if (n->arg)
      res = convert_protocol_argument(n->arg);
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_protocol_specifier(struct protocol_specifier_s *n,
                                          struct filtergen_opts *o) {
  struct filter *res = NULL;

  eprint("converting protocol specifier\n");

  if (n->list) {
    res = new_filter_sibs(convert_protocol_argument_list(n->list, o));
  } else {
    filter_error(n->pos, "no protocol argument list");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_port_argument(struct port_argument_s *n, int type) {
  struct filter *res = NULL;
  char *p;

  eprint("converting port argument\n");

  if (n->port_min) {
    if (n->port_max) {
      if (asprintf(&p, "%s:%s", n->port_min, n->port_max) < 0) {
        filter_error(
            n->pos,
            "error: asprintf allocation failed when emitting port range "
            "%s:%s\n",
            n->port_min, n->port_max);
      }
    } else {
      p = n->port_min;
    }
    res = new_filter_ports(type, p, n->pos);
  } else {
    filter_error(n->pos, "no port argument contents");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_port_argument_list(struct port_argument_list_s *n,
                                          int type, struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting port argument list\n");

  if (n->list) {
    res = convert_port_argument_list(n->list, type, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
      if (n->arg) {
        end->next = convert_port_argument(n->arg, type);
      }
    } else {
      filter_error(n->pos,
                   "warning: convert_port_argument_list returned NULL\n");
    }
  } else {
    if (n->arg)
      res = convert_port_argument(n->arg, type);
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_port_specifier(struct port_specifier_s *n,
                                      struct filtergen_opts *o) {
  struct filter *res = NULL;
  enum filtertype type;

  eprint("converting port specifier\n");

  switch (n->type) {
  case TOK_SPORT:
    type = F_SPORT;
    break;
  case TOK_DPORT:
    type = F_DPORT;
    break;
  default:
    filter_error(n->pos, "incorrect port type encountered");
    type = YYEOF;
    break;
  }
  if (n->list) {
    res = new_filter_sibs(convert_port_argument_list(n->list, type, o));
  } else {
    filter_error(n->pos, "no port argument list");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_icmptype_argument(struct icmptype_argument_s *n) {
  struct filter *res = NULL;

  eprint("converting icmptype_argument\n");

  if (n->icmptype) {
    res = new_filter_icmp(F_ICMPTYPE, n->icmptype, n->pos);
  } else {
    filter_error(n->pos, "no icmptype argument contents");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *
convert_icmptype_argument_list(struct icmptype_argument_list_s *n,
                               struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting icmptype argument list\n");

  if (n->list) {
    res = convert_icmptype_argument_list(n->list, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
      if (n->arg) {
        end->next = convert_icmptype_argument(n->arg);
      }
    } else {
      filter_error(n->pos,
                   "warning: convert_icmptype_argument_list returned NULL\n");
    }
  } else {
    if (n->arg)
      res = convert_icmptype_argument(n->arg);
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_icmptype_specifier(struct icmptype_specifier_s *n,
                                          struct filtergen_opts *o) {
  struct filter *res = NULL;

  eprint("converting icmptype specifier\n");

  if (n->list) {
    res = new_filter_sibs(convert_icmptype_argument_list(n->list, o));
  } else {
    filter_error(n->pos, "no icmptype argument list");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_option_specifier(struct option_specifier_s *n) {
  struct filter *res = NULL;

  eprint("converting option specifier\n");

  switch (n->type) {
  case TOK_LOCAL:
    res = new_filter_rtype(LOCALONLY, n->pos);
    break;
  case TOK_FORWARD:
    res = new_filter_rtype(ROUTEDONLY, n->pos);
    break;
  case TOK_ONEWAY:
    res = new_filter_rtype(F_ONEWAY, n->pos);
    break;
  case TOK_LOG:
    res = new_filter_log(F_LOG, n->logmsg, n->pos);
    break;
  default:
    filter_error(n->pos, "incorrect option type encountered");
    break;
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_chaingroup_specifier(struct chaingroup_specifier_s *n,
                                            struct filtergen_opts *o) {
  struct filter *res = NULL, *sub = NULL;
  char *name = NULL;

  if (n->name) {
    name = n->name;
  } else {
    /* Allocate a filter name */
    static int ccount = 0;

    if (asprintf(&name, "chain_%d", ccount++) < 0) {
      filter_error(
          n->pos,
          "error: asprintf allocation failed when creating a filter name "
          "for chain %d\n",
          ccount);
    }
  }

  if (n->list) {
    sub = convert_subrule_list(n->list, o);
    if (sub) {
      res = new_filter_subgroup(name, sub);
    }
  } else {
    filter_error(n->pos, "no list in chaingroup");
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_specifier(struct specifier_s *r,
                                 struct filtergen_opts *o) {
  struct filter *res = NULL;
  eprint("converting specifier\n");

  if (r->compound) {
    eprint("converting compound specifier\n");
    res = convert_compound_specifier(r->compound, o);
  } else if (r->direction) {
    res = convert_direction(r->direction, o);
  } else if (r->target) {
    enum filtertype type;

    eprint("converting target specifier\n");

    switch (r->target->type) {
    case TOK_ACCEPT:
      type = T_ACCEPT;
      break;
    case TOK_REJECT:
      type = T_REJECT;
      break;
    case TOK_DROP:
      type = DROP;
      break;
    case TOK_MASQ:
      type = MASQ;
      break;
    case TOK_PROXY:
      type = REDIRECT;
      break;
    case TOK_REDIRECT:
      type = REDIRECT;
      break;
    default:
      filter_error(r->pos, "incorrect target type encountered");
      type = YYEOF;
      break;
    }
    res = new_filter_target(type, r->pos);
  } else if (r->host) {
    res = convert_host_specifier(r->host, o);
  } else if (r->protocol) {
    res = convert_protocol_specifier(r->protocol, o);
  } else if (r->port) {
    res = convert_port_specifier(r->port, o);
  } else if (r->icmptype) {
    res = convert_icmptype_specifier(r->icmptype, o);
  } else if (r->option) {
    res = convert_option_specifier(r->option);
  } else if (r->chaingroup) {
    res = convert_chaingroup_specifier(r->chaingroup, o);
  } else {
    filter_error(r->pos, "no specifiers");
  }
  if (res)
    res->pos = r->pos;
  free(r);
  return res;
}

struct filter *convert_negated_specifier(struct negated_specifier_s *r,
                                         struct filtergen_opts *o) {
  struct filter *spec = NULL;
  struct filter *res = NULL;

  eprint("converting negated specifier\n");

  if (r->spec) {
    spec = convert_specifier(r->spec, o);
    if (spec && r->negated) {
      eprint("negating\n");
      res = new_filter_neg(spec);
    } else {
      res = spec;
    }
  }
  if (res)
    res->pos = r->pos;
  free(r);
  return res;
}

struct filter *convert_specifier_list(struct specifier_list_s *n,
                                      struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting specifier_list\n");

  if (n->list) {
    res = convert_specifier_list(n->list, o);
    if (res) {
      end = res;
      while (end->child) {
        end = end->child;
      }
      if (n->spec) {
        end->child = convert_negated_specifier(n->spec, o);
      }
    } else {
      filter_error(n->pos, "warning: convert_specifier_list returned NULL");
    }
  } else {
    if (n->spec)
      res = convert_negated_specifier(n->spec, o);
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert_rule(struct rule_s *r, struct filtergen_opts *o) {
  struct filter *res = NULL;

  eprint("converting rule\n");

  if (r->list)
    res = convert_specifier_list(r->list, o);
  if (res)
    res->pos = r->pos;
  free(r);
  return res;
}

struct filter *convert_rule_list(struct rule_list_s *n,
                                 struct filtergen_opts *o) {
  struct filter *res = NULL, *end = NULL;

  eprint("converting rule_list\n");

  if (n->list) {
    res = convert_rule_list(n->list, o);
    if (res) {
      end = res;
      while (end->next) {
        end = end->next;
      }
    }
  }
  if (n->rule) {
    if (end) {
      end->next = convert_rule(n->rule, o);
    } else {
      res = convert_rule(n->rule, o);
    }
  }
  if (res)
    res->pos = n->pos;
  free(n);
  return res;
}

struct filter *convert(struct ast_s *ast, struct filtergen_opts *o) {
  struct filter *res = NULL;

  eprint("converting ast\n");

  if (ast->list) {
    res = convert_rule_list(ast->list, o);
  }
  if (res)
    res->pos = ast->pos;

  return res;
}

struct filter *filtergen_source_parser(const char *filename, int flags,
                                       struct filtergen_opts *o) {
  struct ast_s ast;
  struct filter *f = NULL;

  filtergen_set_debug(0);

  struct sourceposition pos;
  if (!sourcefile_push(&pos, filename)) {
    return NULL;
  }
  filtergen_in = current_srcfile->f;
  if (filtergen_parse(&ast) == 0) {
    if (!(flags & FF_NORESOLVE)) {
      resolve(&ast, o);
    }
    f = convert(&ast, o);
  } else {
    fprintf(stderr, "couldn't parse file: %s\n", filename);
  }
  return f;
}
