/* conversion glue between iptables-save ast and filtergen internal representation
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
#include <string.h>
#include "filter.h"
#include "input/input.h"
#include "ast.h"
#include "parser.h"

int ipts_parse(void *);
int ipts_restart(FILE *);

int ipts_convtrace = 1;

#define eprint(x) if (ipts_convtrace) fprintf(stderr, x)

void ipts_convert_identifier(struct identifier_s * n) {

  eprint("converting identifier\n");

  if (n->id1) {
    /* do soemthing with first arg */
  }
  if (n->id2) {
    /* do something with second arg */
  }
}

void ipts_convert_not_identifier(struct not_identifier_s * n) {
  eprint("converting not_identifier\n");

  if (n->neg) {
    /* neg is a boolean */
  }
  if (n->identifier) {
    ipts_convert_identifier(n->identifier);
  }
}

struct filter * ipts_convert_in_interface_option(struct in_interface_option_s * n) {
  struct filter * res = NULL;

  eprint("converting in_interface_option\n");

  if (n->not_identifier) {
    ipts_convert_not_identifier(n->not_identifier);
  }

  return res;
}

struct filter * ipts_convert_jump_option(struct jump_option_s * n) {
  struct filter * res = NULL;

  eprint("converting jump_option\n");

  if (n->identifier) {
    ipts_convert_identifier(n->identifier);
  }

  return res;
}

struct filter * ipts_convert_option(struct option_s * n) {
  struct filter * res = NULL;

  eprint("converting option\n");
  
  if (n->in_interface_option) {
    ipts_convert_in_interface_option(n->in_interface_option);
  } else if (n->jump_option) {
    ipts_convert_jump_option(n->jump_option);
  }

  return res;
}

struct filter * ipts_convert_not_option(struct not_option_s * n) {
  struct filter * res = NULL, * opt = NULL;

  eprint("converting not_option\n");

  if (n->option) {
    opt = ipts_convert_option(n->option);
    if (opt && n->neg) {
      res = new_filter_neg(opt);
    } else {
      res = opt;
    }
  }

  return res;
}

struct filter * ipts_convert_option_list(struct option_list_s * n) {
  struct filter * res = NULL, * end = NULL;

  eprint("converting option_list\n");

  if (n->option_list) {
    res = ipts_convert_option_list(n->option_list);
    if (res) {
      end = res;
      while (end->child) {
	end = end->child;
      }
      if (n->not_option) {
	end->child = ipts_convert_not_option(n->not_option);
      }
    } else {
      fprintf(stderr, "warning: ipts_convert_option_list returned NULL\n");
    }
  } else {
    res = ipts_convert_not_option(n->not_option);
  }

  return res;
}

/* FIXME: this function doesn't cope with non-standard chain names
 * and the filter structure can't cope with default chain policies without
 * a device */
struct filter * ipts_convert_rule(struct rule_s * n) {
  struct filter * res = NULL;

  eprint("converting rule\n");

  if (n->chain) {
    /* do something with the chain declaration */
    /* chain, policy, pkt_count are set */
    /* FIXME: somehow append the chain default policy to the end of the
     * rule list */
    int direction;

    if (!strcasecmp(n->chain, "input")) {
      direction = INPUT;
    } else if (!strcasecmp(n->chain, "output")) {
      direction = OUTPUT;
    } else {
      fprintf(stderr, "warning: unhandled chain name %s\n", n->chain);
    }
    res = new_filter_device(direction, "eth0");

    if (n->policy) {
      enum filtertype type;
      if (!strcasecmp(n->policy, "accept")) {
	type = T_ACCEPT;
      } else if (!strcasecmp(n->policy, "drop")) {
	type = DROP;
      } else if (!strcasecmp(n->policy, "reject")) {
	type = T_REJECT;
      } else {
	fprintf(stderr, "warning: invalid chain policy %s\n", n->policy);
	type = YYEOF;
      }
      res->child = new_filter_target(type);
    }
  } else if (n->option_list) {
    /* do something with the option list */
    /* option list, and optionally pkt_count, are set */
    res = ipts_convert_option_list(n->option_list);
  }

  return res;
}

struct filter * ipts_convert_rule_list(struct rule_list_s * n) {
  struct filter * res = NULL, * end = NULL;

    eprint("converting rule list\n");

    if (n->list) {
	res = ipts_convert_rule_list(n->list);
	if (res) {
	  end = res;
	  while (end->next) {
	    end = end->next;
	  }
	  if (n->rule) {
	    end->next = ipts_convert_rule(n->rule);
	  }
	} else {
	  fprintf(stderr, "warning: ipts_convert_rule_list returned NULL\n");
	}
    } else {
      res = ipts_convert_rule(n->rule);
      if (!res) {
	fprintf(stderr, "warning: going to return NULL\n");
      }
    }

    return res;
}

struct filter * ipts_convert_table(struct table_s * n) {
  struct filter * res = NULL;

  eprint("converting table\n");

  if (n->rule_list)
    res = ipts_convert_rule_list(n->rule_list);

  return res;
}

struct filter * ipts_convert_table_list(struct table_list_s * n) {
  struct filter * res = NULL;

  eprint("converting table_list\n");

  if (n->list)
    res = ipts_convert_table_list(n->list);
  if (n->table)
    res = ipts_convert_table(n->table);

  return res;
}

struct filter * ipts_convert(struct ast_s * ast) {
  struct filter * res = NULL;

    eprint("converting ast\n");

    if (ast->list)
      res = ipts_convert_table_list(ast->list);

    return res;
}

struct filter * ipts_source_parser(FILE * file, int resolve_names __attribute__((unused))) {
    struct ast_s ast;
    struct filter * f = NULL;

    ipts_restart(file);
    if (ipts_parse((void *) &ast) != 0) {
	fprintf(stderr, "iptables-save parse failed\n");
	f = NULL;
    } else {
	if ((f = ipts_convert(&ast)) == NULL) {
	    fprintf(stderr, "iptables-save conversion failed\n");
	}
    }
    return f;
}
