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

void ipts_convert_option(struct option_s * n) {
  eprint("converting option\n");

  if (n->option) {
    /* option is a string representing the option */
  }
  if (n->not_identifier) {
    ipts_convert_not_identifier(n->not_identifier);
  }
}

void ipts_convert_not_option(struct not_option_s * n) {
  eprint("converting not_option\n");

  if (n->neg) {
    /* neg is a boolean */
  }
  if (n->option) {
    ipts_convert_option(n->option);
  }
}

void ipts_convert_option_list(struct option_list_s * n) {
  eprint("converting option_list\n");

  if (n->option_list) {
    ipts_convert_option_list(n->option_list);
  }
  if (n->not_option) {
    ipts_convert_not_option(n->not_option);
  }
}

struct filter * ipts_convert_rule(struct rule_s * n) {
  struct filter * res = NULL;

  eprint("converting rule\n");

  if (n->table) {
    /* do something with the table declaration */
    /* nat vs filter */
  } else if (n->chain) {
    /* do something with the chain declaration */
    /* chain, policy, pkt_count are set */
  } else if (n->option_list) {
    /* do something with the option list */
    /* option list, and optionally pkt_count, are set */
    ipts_convert_option_list(n->option_list);
  }

  return res;
}

struct filter * ipts_convert_rule_list(struct rule_list_s * n) {
    struct filter * res = NULL;
    struct filter * r = NULL;

    eprint("converting rule list\n");

    if (n->list) {
	res = ipts_convert_rule_list(n->list);
    }
    if (n->rule) {
      r = ipts_convert_rule(n->rule);
    }
      
    return res;
}

struct filter * ipts_convert(struct ast_s * ast) {
  struct filter * res = NULL;

    eprint("converting ast\n");

    if (ast->list)
      res = ipts_convert_rule_list(ast->list);

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
