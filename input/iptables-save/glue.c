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
#include <assert.h>
#include "ir/ir.h"
#include "input/input.h"
#include "ast.h"
#include "parser.h"

int ipts_parse(void *);
int ipts_restart(FILE *);

int ipts_convtrace = 1;

#define eprint(x) if (ipts_convtrace) fprintf(stderr, x)

#define CONVERT(x) int ipts_convert_##x(struct x##_s * n)

int ipts_convert_identifier(struct identifier_s * n, struct ir_value_s * ir_value) {
    int res = 0;
    
    eprint("converting identifier\n");

    assert(ir_value);

    if (n->string) {
	/*printf("%s\n", n->string);*/
	ir_value->type = IR_VAL_LITERAL;
	ir_value->u.value = n->string;
	res = 1;
    }

    return res;
}

int ipts_convert_not_identifier(struct not_identifier_s * n, struct ir_expr_s * ir_expr) {
    int res = 0;
    /* temporary reference value */
    struct ir_expr_s * e;
    
    eprint("converting not_identifier\n");

    assert(ir_expr);

    e = ir_expr;

    /* initialise the value store */
    ir_expr->value = ir_value_new();
    
    if (n->neg) {
	ir_expr->value->type = IR_VAL_OPERATOR;
	ir_expr->value->u.operator = IR_OP_NOT;

	ir_expr->left = ir_expr_new();
	e = ir_expr->left;
    }
    if (n->identifier) {
	res = ipts_convert_identifier(n->identifier, e->value);
    }

    return 0;
}

#if 0
int ipts_convert_range(struct range_s * n, struct ir_expr_s * ir_expr) {
    eprint("converting range\n");

    assert(ir_expr);

    if (n->start) {
	ipts_convert_identifier(n->start, ir_expr);
    }
    if (n->end) {
	ipts_convert_identifier(n->end, ir_expr);
    }

    return 1;
}

int ipts_convert_not_range(struct not_range_s * n, struct ir_expr_s * ir_expr) {
    eprint("converting not_range\n");

    assert(ir_expr);

    if (n->neg) {
	/* neg is boolean */
    }
    if (n->range) {
	ipts_convert_range(n->range, ir_expr);
    }

    return 1;
}
#endif

int ipts_convert_in_interface_option(struct in_interface_option_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;

    eprint("converting in_interface_option\n");

    assert(ir_expr);

    if (n->not_identifier) {
	ipts_convert_not_identifier(n->not_identifier, ir_expr);
    }

    return res;
}

int ipts_convert_jump_option(struct jump_option_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting jump_option\n");

    assert(ir_rule);
    assert(n->identifier);

    ir_rule->action = ir_action_new();

    assert(n->identifier->string);
    
    if (!strcasecmp(n->identifier->string, "accept")) {
	ir_rule->action->type = IR_ACCEPT;
    } else if (!strcasecmp(n->identifier->string, "drop")) {
	ir_rule->action->type = IR_DROP;
    } else if (!strcasecmp(n->identifier->string, "reject")) {
	ir_rule->action->type = IR_REJECT;
    } else if (!strcasecmp(n->identifier->string, "log")) {
	ir_rule->action->type = IR_LOG;
    } else {
	fprintf(stderr, "warning: jump argument %s not handled\n", n->identifier->string);
    }

    return res;
}

int ipts_convert_option(struct option_s * n, struct ir_rule_s * ir_rule) {
  int res = 1;
  struct ir_expr_s * e;

  eprint("converting option\n");

  if (n->jump_option) {
      eprint("setting rule action\n");
      res = ipts_convert_jump_option(n->jump_option, ir_rule);
  } else {
      if (ir_rule->expr) {
	  eprint("shifting expr with new root AND\n");
	  e = ir_expr_new();
	  e->value = ir_value_new();
	  e->value->type = IR_VAL_OPERATOR;
	  e->value->u.operator = IR_OP_AND;

	  e->left = ir_rule->expr;
	  e->right = ir_expr_new();
	  ir_rule->expr = e;
	  
	  e = e->right;

      } else {
	  eprint("creating new root expr\n");
	  ir_rule->expr = ir_expr_new();
	  e = ir_rule->expr;
      }

      if (n->in_interface_option) {
	  eprint("going to convert in_interface option\n");
	  res = ipts_convert_in_interface_option(n->in_interface_option, e);
      } else if (n->source_option) {
	  eprint("going to convert source option\n");
      }	  
  }

  return res;
}

int ipts_convert_not_option(struct not_option_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting not_option\n");

    if (n->neg) {
	/* create not operator */
    }

    if (n->option) {
	res = ipts_convert_option(n->option, ir_rule);
    }

    return res;
}

int ipts_convert_option_list(struct option_list_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting option_list\n");

    assert(ir_rule);

    if (n->option_list) {
	res = ipts_convert_option_list(n->option_list, ir_rule);
    }
    if (n->not_option)
	res = ipts_convert_not_option(n->not_option, ir_rule);

    return res;
}

/* FIXME: this function doesn't cope with non-standard chain names
 * and the filter structure can't cope with default chain policies without
 * a device */
int ipts_convert_rule(struct rule_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting rule\n");

    assert(ir_rule);

    if (n->policy) {
	/* do something with the chain declaration */
	/* chain, policy, pkt_count are set */
	/* FIXME: somehow append the chain default policy to the end of the
	 * rule list */
	eprint("ignoring default chain policy\n");
    } else if (n->option_list) {
	/* do something with the option list */
	/* option list, and optionally pkt_count, are set */
	res = ipts_convert_option_list(n->option_list, ir_rule);
    }

    return res;
}

int ipts_convert_rule_list(struct rule_list_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting rule list\n");

    assert(ir_rule);

    res = ipts_convert_rule(n->rule, ir_rule);

    if (n->list) {
	ir_rule->next = ir_rule_new();
	res = ipts_convert_rule_list(n->list, ir_rule->next);
    }

    return res;
}

int ipts_convert_table(struct table_s * n, struct ir_s * ir) {
  int res = 1;

  eprint("converting table\n");

  assert(ir);

  if (!strcasecmp(n->name, "filter")) {
      ir->filter = ir_rule_new();
      res = ipts_convert_rule_list(n->rule_list, ir->filter);
  } else {
      fprintf(stderr, "warning: not handling table name %s\n", n->name);
  }

  return res;
}

int ipts_convert_table_list(struct table_list_s * n, struct ir_s * ir) {
    int res = 1;
    
    eprint("converting table_list\n");

    if (n->list)
	res = ipts_convert_table_list(n->list, ir);
    
    if (n->table)
	res = ipts_convert_table(n->table, ir);

    return res;
}

int ipts_convert(struct ast_s * ast, struct ir_s * ir) {
    int res = 1;
    
    eprint("converting ast\n");

    if (ast->list)
	res = ipts_convert_table_list(ast->list, ir);

    return res;
}

struct ir_s * ipts_source_parser(FILE * file, int resolve_names __attribute__((unused))) {
    struct ast_s ast;
    struct ir_s * ir;

    ir = ir_new();

    ipts_restart(file);
    if (ipts_parse((void *) &ast) != 0) {
	fprintf(stderr, "iptables-save parse failed\n");
    } else {
	if (!ipts_convert(&ast, ir)) {
	    fprintf(stderr, "iptables-save conversion failed\n");
	}
    }
    return ir;
}
