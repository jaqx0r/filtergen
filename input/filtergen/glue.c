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
#include <string.h>
#include <assert.h>
#include "input/input.h"
#include "ir/ir.h"
#include "ast.h"
#include "parser.h"
#include "resolver.h"

int filtergen_parse(void *);
int filtergen_restart(FILE *);

int convtrace = 1;

#define eprint(x) if (convtrace) fprintf(stderr, x)

#if 0

struct ir_s * filtergen_convert_specifier_list(struct specifier_list_s * n);

struct ir_s * filtergen_convert_subrule_list(struct subrule_list_s * n) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting subrule_list\n");

    if (n->subrule_list) {
	res = filtergen_convert_subrule_list(n->subrule_list);
	if (res) {
	    end = res;
	    while (end->next) {
		end = end->next;
	    }
	    if (n->specifier_list) {
		end->next = filtergen_convert_specifier_list(n->specifier_list);
	    }
	} else {
	    printf("warning: filtergen_convert_subrule_list returned NULL\n");
	}
    } else if (n->specifier_list) {
        res = filtergen_convert_specifier_list(n->specifier_list);
    } else {
        printf("error: no content in subrule_list\n");
    }

    return res;
}

struct ir_s * filtergen_convert_compound_specifier(struct compound_specifier_s * r) {
    struct ir_s * res = NULL;

    eprint("filtergen_converting compound_specifier\n");

    if (r->list) {
	res = new_filter_sibs(filtergen_convert_subrule_list(r->list));
    }
    return res;
}
#endif

struct ir_expr_s * filtergen_convert_direction_argument(struct direction_argument_s * n) {
    struct ir_expr_s * ir_expr = NULL;

    assert(n);

    if (n->direction) {
	ir_expr = ir_expr_new_predicate("interface");
	ir_expr->left = ir_expr_new_literal(n->direction);
    } else {
        printf("error: no direction argument contents\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_direction_argument_list(struct direction_argument_list_s * n) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting direction argument list\n");

    assert(n);

    ir_expr = filtergen_convert_direction_argument(n->arg);

    if (n->list) {
	struct ir_expr_s * e;

	e = filtergen_convert_direction_argument_list(n->list);

	if (ir_expr) {
	    struct ir_expr_s * o;

	    o = ir_expr_new_operator(IR_OP_OR);

	    o->left = ir_expr;

	    o->right = e;

	    ir_expr = o;
	} else {
	    ir_expr = e;
	}
    }

    return ir_expr;
}
    
struct ir_expr_s * filtergen_convert_direction(struct direction_specifier_s * n) {
    struct ir_expr_s * ir_expr = NULL;
    
    eprint("filtergen_converting direction specifier\n");

    assert(n);

    ir_expr = ir_expr_new_operator(IR_OP_AND);
    ir_expr->left = ir_expr_new_predicate("direction");

    switch (n->type) {
      case TOK_INPUT:
	ir_expr->left->left = ir_expr_new_literal("INPUT");
	break;
      case TOK_OUTPUT:
	ir_expr->left->left = ir_expr_new_literal("OUTPUT");
	break;
      default:
	printf("error: incorrect direction type encountered\n");
	break;
    }

    if (n->list) {
	ir_expr->right = filtergen_convert_direction_argument_list(n->list);
    } else {
	printf("error: no direction argument list\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_host_argument(struct host_argument_s * n, const char * predicate) {
    struct ir_expr_s * ir_expr = NULL;
    char * h;

    eprint("filtergen_converting host_argument\n");

    assert(n);

    if (n->host) {
	ir_expr = ir_expr_new_predicate(predicate);
	
        if (n->mask) {
	    asprintf(&h, "%s/%s", n->host, n->mask);
	    ir_expr->left = ir_expr_new_literal(h);
	} else {
	    ir_expr->left = ir_expr_new_literal(n->host);
	}
    } else {
        printf("error: no host part\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_host_argument_list(struct host_argument_list_s * n, const char * predicate) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting host argument list\n");

    assert(n);

    ir_expr = filtergen_convert_host_argument(n->arg, predicate);

    if (n->list) {
	if (ir_expr) {
	    struct ir_expr_s * o;

	    o = ir_expr_new_operator(IR_OP_OR);

	    o->left = ir_expr;

	    o->right = filtergen_convert_host_argument_list(n->list, predicate);

	    ir_expr = o;
	} else {
	    ir_expr = filtergen_convert_host_argument_list(n->list, predicate);
	}
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_host_specifier(struct host_specifier_s * n) {
    struct ir_expr_s * ir_expr = NULL;
    char * predicate = NULL;
	
    eprint("filtergen_converting host specifier\n");

    assert(n);

    switch (n->type) {
      case TOK_SOURCE:
	predicate = strdup("source");
        break;
      case TOK_DEST:
	predicate = strdup("destination");
        break;
      default:
        fprintf(stderr, "error: incorrect host type encountered\n");
        break;
    }
    
    if (n->list) {
	if (predicate)
	    ir_expr = filtergen_convert_host_argument_list(n->list, predicate);
    } else {
	fprintf(stderr, "error: no host argument list\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_protocol_argument(struct protocol_argument_s * n) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting protocol argument\n");

    assert(n);

    if (n->proto) {
	ir_expr = ir_expr_new_predicate("protocol");
	ir_expr->left = ir_expr_new_literal(n->proto);
    } else {
        printf("error: no protocol argument contents\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_protocol_argument_list(struct protocol_argument_list_s * n) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting protocol argument list\n");

    assert(n);

    if (n->arg) {
	ir_expr = filtergen_convert_protocol_argument(n->arg);
    }

    if (n->list) {
	struct ir_expr_s * e;

	e = filtergen_convert_protocol_argument_list(n->list);

	if (ir_expr) {
	    struct ir_expr_s * o;

	    o = ir_expr_new_operator(IR_OP_OR);

	    o->left = ir_expr;
	    o->right = e;

	    ir_expr = o;
	} else {
	    ir_expr = e;
	}
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_protocol_specifier(struct protocol_specifier_s * n) {
    struct ir_expr_s * ir_expr = NULL;
	
    eprint("filtergen_converting protocol specifier\n");

    assert(n);

    if (n->list) {
	ir_expr = filtergen_convert_protocol_argument_list(n->list);
    } else {
	printf("error: no protocol argument list\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_port_argument(struct port_argument_s * n, const char * predicate) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting port argument\n");

    assert(n);

    if (n->port_min) {
	ir_expr = ir_expr_new_predicate(predicate);

	ir_expr->left = ir_expr_new_literal(n->port_min);

        if (n->port_max) {
	    struct ir_expr_s * e;

	    e = ir_expr_new_range();

	    e->left = ir_expr->left;
	    e->right = ir_expr_new_literal(n->port_max);

	    ir_expr->left = e;
        }
    } else {
        printf("error: no port argument contents\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_port_argument_list(struct port_argument_list_s * n, const char * predicate) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting port argument list\n");

    assert(n);

    if (n->arg) {
	ir_expr = filtergen_convert_port_argument(n->arg, predicate);
    }

    if (n->list) {
	struct ir_expr_s * e;

	e = filtergen_convert_port_argument_list(n->list, predicate);

	if (ir_expr) {
	    struct ir_expr_s * o;

	    o = ir_expr_new_operator(IR_OP_OR);
	    o->left = ir_expr;
	    o->right = e;

	    ir_expr = o;
	} else {
	    ir_expr = e;
	}
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_port_specifier(struct port_specifier_s * n) {
    struct ir_expr_s * ir_expr = NULL;
    char * predicate = NULL;
	
    eprint("filtergen_converting port specifier\n");

    assert(n);

    switch (n->type) {
      case TOK_SPORT:
	predicate = strdup("sport");
        break;
      case TOK_DPORT:
	predicate = strdup("dport");
        break;
      default:
        printf("error: incorrect port type encountered\n");
        break;
    }

    if (n->list) {
	ir_expr = filtergen_convert_port_argument_list(n->list, predicate);
    } else {
	printf("error: no port argument list\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_icmptype_argument(struct icmptype_argument_s * n) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting icmptype_argument\n");

    assert(n);

    if (n->icmptype) {
	ir_expr = ir_expr_new_predicate("icmptype");
	ir_expr->left = ir_expr_new_literal(n->icmptype);
    } else {
        printf("error: no icmptype argument contents\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_icmptype_argument_list(struct icmptype_argument_list_s * n) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting icmptype argument list\n");

    assert(n);

    if (n->arg) {
	ir_expr = filtergen_convert_icmptype_argument(n->arg);
    }

    if (n->list) {
	struct ir_expr_s * e;

	e = filtergen_convert_icmptype_argument_list(n->list);

	if (ir_expr) {
	    struct ir_expr_s * o;

	    o = ir_expr_new_operator(IR_OP_OR);
	    o->left = ir_expr;
	    o->right = e;

	    ir_expr = o;
	} else {
	    ir_expr = e;
	}
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_icmptype_specifier(struct icmptype_specifier_s * n) {
    struct ir_expr_s * ir_expr = NULL;
	
    eprint("filtergen_converting icmptype specifier\n");

    assert(n);

    if (n->list) {
	ir_expr = filtergen_convert_icmptype_argument_list(n->list);
    } else {
	printf("error: no icmptype argument list\n");
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_option_specifier(struct option_specifier_s * n) {
    struct ir_expr_s * ir_expr = NULL;
	
    eprint("filtergen_converting option specifier\n");

    assert(n);

    switch (n->type) {
      case TOK_LOCAL:
	ir_expr = ir_expr_new_predicate("localonly");
	break;
      case TOK_FORWARD:
	ir_expr = ir_expr_new_predicate("forward");
	break;
      case TOK_ONEWAY:
	ir_expr = ir_expr_new_predicate("oneway");
	break;
      case TOK_LOG:
	ir_expr = ir_expr_new_predicate("log");
	ir_expr->left = ir_expr_new_literal(n->logmsg);
	break;
      default:
	printf("error: incorrect option type encountered\n");
	break;
    }

    return ir_expr;
}

#if 0
struct ir_s * filtergen_convert_chaingroup_specifier(struct chaingroup_specifier_s * n) {
    struct ir_s * res = NULL, * sub = NULL;
    char * name = NULL;

    if (n->name) {
	name = n->name;
    } else {
	/* Allocate a filter name */
	static int ccount = 0;

	asprintf(&name, "chain_%d", ccount++);
    }
  
    if (n->list) {
	sub = filtergen_convert_subrule_list(n->list);

	res = new_filter_subgroup(name, sub);
    } else {
	printf("error: no list in chaingroup\n");
    }

    return res;
}
#endif

struct ir_expr_s * filtergen_convert_specifier(struct specifier_s * r, struct ir_rule_s * ir_rule) {
    struct ir_expr_s * ir_expr = NULL;
    
    eprint("filtergen_converting specifier\n");

    assert(r);
    
    if (r->compound) {
        eprint("filtergen_converting compound specifier\n");
	/*res = filtergen_convert_compound_specifier(r->compound);*/
    } else if (r->direction) {
        ir_expr = filtergen_convert_direction(r->direction);
    } else if (r->target) {

	eprint("filtergen_converting target specifier\n");
	
	assert(ir_rule);

	ir_rule->action = ir_action_new();

	switch (r->target->type) {
	  case TOK_ACCEPT:
	    ir_rule->action->type = IR_ACCEPT;
	    break;
	  case TOK_REJECT:
	    ir_rule->action->type = IR_REJECT;
	    break;
	  case TOK_DROP:
	    ir_rule->action->type = IR_DROP;
	    break;
	  case TOK_MASQ:
	    ir_rule->action->type = IR_ACCEPT;
	    break;
	  case TOK_PROXY:
	    ir_rule->action->type = IR_ACCEPT;
	    break;
	  case TOK_REDIRECT:
	    ir_rule->action->type = IR_ACCEPT;
	    break;
	  default:
	    printf("error: incorrect target type encountered\n");
	    break;
	}
    } else if (r->host) {
        ir_expr = filtergen_convert_host_specifier(r->host);
    } else if (r->protocol) {
        ir_expr = filtergen_convert_protocol_specifier(r->protocol);
    } else if (r->port) {
        ir_expr = filtergen_convert_port_specifier(r->port);
    } else if (r->icmptype) {
        ir_expr = filtergen_convert_icmptype_specifier(r->icmptype);
    } else if (r->option) {
	ir_expr = filtergen_convert_option_specifier(r->option);
    } else if (r->chaingroup) {
	/*
	res = filtergen_convert_chaingroup_specifier(r->chaingroup);
	*/
    } else
	printf("error: no specifiers\n");
    
    return ir_expr;
}

struct ir_expr_s * filtergen_convert_negated_specifier(struct negated_specifier_s * r, struct ir_rule_s * ir_rule) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting negated specifier\n");

    assert(r);
    assert(ir_rule);

    if (r->spec) {
	ir_expr = filtergen_convert_specifier(r->spec, ir_rule);

	if (ir_expr && r->negated) {
	    struct ir_expr_s * n;

	    n = ir_expr_new_operator(IR_OP_NOT);

	    n->left = ir_expr;

	    ir_expr = n;
	}
    }

    return ir_expr;
}

struct ir_expr_s * filtergen_convert_specifier_list(struct specifier_list_s * n, struct ir_rule_s * ir_rule) {
    struct ir_expr_s * ir_expr = NULL;

    eprint("filtergen_converting specifier_list\n");

    assert(n);
    assert(ir_rule);

    if (n->spec) {
	ir_expr = filtergen_convert_negated_specifier(n->spec, ir_rule);
    }

    if (n->list) {
	struct ir_expr_s * e = NULL;
	
	e = filtergen_convert_specifier_list(n->list, ir_rule);

	if (ir_expr) {
	    struct ir_expr_s * a;

	    a = ir_expr_new_operator(IR_OP_AND);

	    a->left = ir_expr;

	    a->right = e;

	    ir_expr = a;
	} else {
	    ir_expr = e;
	}
    }

    return ir_expr;
}

struct ir_rule_s * filtergen_convert_rule(struct rule_s * r) {
    struct ir_rule_s * ir_rule = NULL;

    eprint("filtergen_converting rule\n");

    assert(r);

    ir_rule = ir_rule_new();

    if (r->list) {
	ir_rule->expr = filtergen_convert_specifier_list(r->list, ir_rule);
    }

    return ir_rule;
}

struct ir_rule_s * filtergen_convert_rule_list(struct rule_list_s * n) {
    struct ir_rule_s * ir_rule = NULL, * r = NULL;

    eprint("filtergen_converting rule_list\n");

    assert(n);

    if (n->rule) {
	ir_rule = filtergen_convert_rule(n->rule);
    }

    if (n->list) {
	r = filtergen_convert_rule_list(n->list);

	if (ir_rule) {
	    ir_rule->next = r;
	} else {
	    ir_rule = r;
	}
    }

    return ir_rule;
}

struct ir_s * filtergen_convert(struct ast_s * ast) {
    struct ir_s * ir = NULL;

    eprint("filtergen_converting ast\n");

    assert(ast);

    ir = ir_new();

    if (ast->list) {
	ir->filter = filtergen_convert_rule_list(ast->list);
    }
    
    return ir;
}

struct ir_s * filtergen_source_parser(FILE * file, int resolve_names) {
    struct ast_s ast;
    struct ir_s * ir;

    filtergen_restart(file);
    
    if (filtergen_parse((void *) &ast) != 0) {
	fprintf(stderr, "filtergen parse failed\n");
    } else {
	
	if (resolve_names)
	    resolve(&ast);
	
	if (!(ir = filtergen_convert(&ast))) {
	    fprintf(stderr, "filtergen IR conversion failed\n");
	}
    }

    return ir;
}
