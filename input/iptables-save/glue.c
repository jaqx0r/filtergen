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

int ipts_convert_identifier(struct identifier_s * n, struct ir_value_s * ir_value) {
    int res = 0;
    
    eprint("converting identifier\n");

    assert(ir_value);

    if (n->string) {
	/*printf("%s\n", n->string);*/
	ir_value->type = IR_VAL_LITERAL;
	ir_value->u.literal = n->string;
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

int ipts_convert_range(struct range_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;
    eprint("converting range\n");

    assert(ir_expr);

    ir_expr->value = ir_value_new();
    ir_expr->value->type = IR_VAL_RANGE;

    if (n->start) {
	ir_expr->left = ir_expr_new();
	ir_expr->left->value = ir_value_new();
	res = ipts_convert_identifier(n->start, ir_expr->left->value);
    }
    if (n->end) {
	ir_expr->right = ir_expr_new();
	ir_expr->right->value = ir_value_new();
	res = ipts_convert_identifier(n->end, ir_expr->right->value);
    }

    return res;
}

int ipts_convert_not_range(struct not_range_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;
    struct ir_expr_s * e;
    
    eprint("converting not_range\n");

    assert(ir_expr);
    e = ir_expr;

    if (n->neg) {
	ir_expr->value = ir_value_new();
	ir_expr->value->type = IR_VAL_OPERATOR;
	ir_expr->value->u.operator = IR_OP_NOT;
	ir_expr->left = ir_expr_new();
	e = ir_expr->left;
    }
    if (n->range) {
	res = ipts_convert_range(n->range, e);
    }

    return res;
}

int ipts_convert_jump_option(struct option_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting jump_option\n");

    assert(ir_rule);
    assert(n->identifier);

    if (ir_rule->action)
	fprintf(stderr, "warning: rule already has an action\n");
    
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

/**
 * convert an ipts option into a predicate in the internal representation
 * @param n the ipts option struct
 * @param ir_rule the internal represenation of the rule this option belongs to
 */
int ipts_convert_option(struct option_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;
    struct ir_expr_s * e;

    eprint("converting option\n");

    assert(n);

    if (n->type == IPTS_OPT_JUMP) {
	eprint("setting rule action\n");
	res = ipts_convert_jump_option(n, ir_rule);
    } else if (n->type == IPTS_OPT_LOG_PREFIX) {
	if (!ir_rule->action)
	    ir_rule->action = ir_action_new();
	ir_rule->action->option = ir_expr_new();
	ir_rule->action->option->value = ir_value_new();
	ir_rule->action->option->value->type = IR_VAL_PREDICATE;
	ir_rule->action->option->value->u.predicate = strdup("log-prefix");
	ir_rule->action->option->left = ir_expr_new();
	ir_rule->action->option->left->value = ir_value_new();
    
	res = ipts_convert_identifier(n->identifier, ir_rule->action->option->left->value);

    } else if (n->type == IPTS_OPT_REJECT_WITH) {
	if (!ir_rule->action)
	    ir_rule->action = ir_action_new();
	ir_rule->action->option = ir_expr_new();
	ir_rule->action->option->value = ir_value_new();
	ir_rule->action->option->value->type = IR_VAL_PREDICATE;
	ir_rule->action->option->value->u.predicate = strdup("reject-with");
	ir_rule->action->option->left = ir_expr_new();
	ir_rule->action->option->left->value = ir_value_new();
    
	res = ipts_convert_identifier(n->identifier, ir_rule->action->option->left->value);
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

	e->value = ir_value_new();
	e->value->type = IR_VAL_PREDICATE;
	e->left = ir_expr_new();

	switch(n->type) {
	  case IPTS_OPT_IN_INTERFACE:
	    eprint("going to convert in_interface option\n");
	    e->value->u.predicate = strdup("in_interface");
	    if (n->not_identifier)
		ipts_convert_not_identifier(n->not_identifier, e->left);
	    break;
	  case IPTS_OPT_OUT_INTERFACE:
	    eprint("going to convert out_interface option\n");
	    e->value->u.predicate = strdup("out_interface");
	    if (n->not_identifier)
		ipts_convert_not_identifier(n->not_identifier, e->left);
	    break;
	  case IPTS_OPT_PROTOCOL:
	    eprint("going to convert protocol option\n");
	    e->value->u.predicate = strdup("protocol");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_SOURCE:
	    eprint("going to convert source option\n");
	    e->value->u.predicate = strdup("source");
	    if (n->not_identifier)
		ipts_convert_not_identifier(n->not_identifier, e->left);
	    break;
	  case IPTS_OPT_DESTINATION:
	    eprint("going to convert destination option\n");
	    e->value->u.predicate = strdup("destination");
	    if (n->not_identifier)
		ipts_convert_not_identifier(n->not_identifier, e->left);
	    break;
	  case IPTS_OPT_MATCH:
	    eprint("going to convert match option\n");
	    e->value->u.predicate = strdup("match");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_LIMIT:
	    eprint("going to convert limit option\n");
	    e->value->u.predicate = strdup("limit");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_STATE:
	    eprint("going to convert state option\n");
	    e->value->u.predicate = strdup("state");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_DPORT:
	    eprint("going to convert dport option\n");
	    e->value->u.predicate = strdup("dport");
	    if (n->not_range) {
		ipts_convert_not_range(n->not_range, e->left);
	    } else if (n->identifier) {
		ipts_convert_not_identifier(n->not_identifier, e->left);
	    } else {
		fprintf(stderr, "warning: no argument to dport in conversion\n");
	    }
	    break;
	  case IPTS_OPT_SPORT:
	    eprint("going to convert sport option\n");
	    e->value->u.predicate = strdup("sport");
	    if (n->not_range) {
		ipts_convert_not_range(n->not_range, e->left);
	    } else if (n->identifier) {
		ipts_convert_not_identifier(n->not_identifier, e->left);
	    } else {
		fprintf(stderr, "warning: no argument to sport in conversion\n");
	    }
	    break;
	  case IPTS_OPT_TO_SOURCE:
	    eprint("going to convert to-source option\n");
	    e->value->u.predicate = strdup("to-source");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_TO_PORTS:
	    eprint("going to convert to-ports option\n");
	    e->value->u.predicate = strdup("to-ports");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_ICMP_TYPE:
	    eprint("going to convert icmp_type option\n");
	    e->value->u.predicate = strdup("icmp_type");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_UID_OWNER:
	    eprint("going to convert uid-owner option\n");
	    e->value->u.predicate = strdup("uid-owner");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_HELPER:
	    eprint("going to convert helper option\n");
	    e->value->u.predicate = strdup("helper");
	    if (n->identifier) {
		e->left->value = ir_value_new();
		ipts_convert_identifier(n->identifier, e->left->value);
	    }
	    break;
	  case IPTS_OPT_TCP_FLAGS:
	    eprint("going to convert tcp_flags option\n");
	    e->value->u.predicate = strdup("tcp_flags");
	    if (n->flags) {
		e->left->value = ir_value_new();
		res = ipts_convert_identifier(n->flags, e->left->value);
	    }
	    if (n->mask) {
		e->right = ir_expr_new();
		e->right->value = ir_value_new();
		res = ipts_convert_identifier(n->mask, e->right->value);
	    }
	    break;
	  case IPTS_OPT_FRAGMENT:
	    eprint("going to convert fragment option\n");
	    e->value->u.predicate = strdup("fragment");
	    break;
	  case IPTS_OPT_SYN:
	    eprint("going to convert syn option\n");
	    e->value->u.predicate = strdup("syn");
	    break;
	  case IPTS_OPT_CLAMP_MSS_TO_PMTU:
	    eprint("going to convert clamp_mss_to_pmtu option\n");
	    e->value->u.predicate = strdup("clamp_mss_to_pmtu");
	    break;
	  default:
	    fprintf(stderr, "warning: unknown option type %d\n", n->type);
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

int ipts_convert_rule(struct rule_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting rule\n");

    assert(ir_rule);

    /* create an expression matching the chain name, aka direction of travel.
     * this is obviously incorrect for nonstandard chain names. */
    if (n->chain) {
	struct ir_expr_s * e;

	e = ir_expr_new();
	e->value = ir_value_new();
	e->value->type = IR_VAL_PREDICATE;
	e->value->u.predicate = strdup("direction");
	e->left = ir_expr_new();
	e->left->value = ir_value_new();
	e->left->value->type = IR_VAL_LITERAL;
	e->left->value->u.literal = strdup(n->chain);

	if (ir_rule->expr) {
	    struct ir_expr_s * a;
	    a = ir_expr_new();
	    a->value = ir_value_new();
	    a->value->type = IR_VAL_OPERATOR;
	    a->value->u.operator = IR_OP_AND;
	    a->left = ir_rule->expr;
	    a->right = e;
	    ir_rule->expr = a;
	} else {
	    ir_rule->expr = e;
	}
    }	    

    if (n->policy) {
	/* do something with the chain declaration */
	/* chain, policy, pkt_count are set */
	ir_rule->action = ir_action_new();
	if (!strcasecmp(n->policy, "drop"))
	    ir_rule->action->type = IR_DROP;
	else if (!strcasecmp(n->policy, "drop"))
	    ir_rule->action->type = IR_DROP;
	else if (!strcasecmp(n->policy, "reject"))
	    ir_rule->action->type = IR_REJECT;
	else
	    fprintf(stderr, "warning: unknown policy %s\n", n->policy);

    } else if (n->option_list) {
	/* do something with the option list */
	/* option list, and optionally pkt_count, are set */
	res = ipts_convert_option_list(n->option_list, ir_rule);
    }

    return res;
}

struct ir_rule_s * ipts_convert_rule_list(struct rule_list_s * n) {
    /* the return value */
    struct ir_rule_s * ir_rule = NULL, * r;
    struct rule_list_s * rl;
    /* this is the default policy rule */
    struct rule_s * def = NULL;
  

    eprint("converting rule list\n");
    
    for (rl = n; rl != NULL; rl = rl->list) {
	if (rl->rule->policy) {
	    /* this rule is the default policy and should go at the end of the IR
	     * rule chain */
	    def = rl->rule;
	} else {
	    r = ir_rule_new();
	    ipts_convert_rule(rl->rule, r);

	    r->next = ir_rule;

	    ir_rule = r;
	}
    }

    if (def) {
	/* put the default policy at the end of the chain */
	for (r = ir_rule; r->next != NULL; r = r->next);
	if (r != ir_rule) {
	    r->next = ir_rule_new();
	    r = r->next;
	}
	ipts_convert_rule(def, r);
    }

    return ir_rule;
}

int ipts_convert_table(struct table_s * n, struct ir_s * ir) {
    int res = 1;

    eprint("converting table\n");

    assert(ir);

    if (!strcasecmp(n->name, "filter")) {
	ir->filter = ipts_convert_rule_list(n->rule_list);
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
