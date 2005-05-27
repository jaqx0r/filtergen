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

#if 0
int ipts_convert_tcp_flags_option(struct tcp_flags_option_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;
    eprint("converting tcp_flags_option\n");
    assert(ir_expr);
    assert(ir_expr->value);
    assert(ir_expr->left);
    ir_expr->value->u.predicate = strdup("tcp_flags");
    if (n->flags) {
	ir_expr->left->value = ir_value_new();
	res = ipts_convert_identifier(n->flags, ir_expr->left->value);
    }
    if (n->mask) {
	ir_expr->right = ir_expr_new();
	ir_expr->right->value = ir_value_new();
	res = ipts_convert_identifier(n->mask, ir_expr->right->value);
    }
    return res;
}

int ipts_convert_fragment_option(struct fragment_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting fragment_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("fragment");
  ir_expr->left->value = ir_value_new();
  ir_expr->left->value->type = IR_VAL_LITERAL;
  asprintf(&ir_expr->left->value->u.literal, "%d", n->i);
  return res;
}

int ipts_convert_syn_option(struct syn_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting syn_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("syn");
  ir_expr->left->value = ir_value_new();
  ir_expr->left->value->type = IR_VAL_LITERAL;
  asprintf(&ir_expr->left->value->u.literal, "%d", n->i);
  return res;
}

int ipts_convert_clamp_mss_to_pmtu_option(struct clamp_mss_to_pmtu_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting clamp_mss_to_pmtu_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("clamp_mss_to_pmtu");
  ir_expr->left->value = ir_value_new();
  ir_expr->left->value->type = IR_VAL_LITERAL;
  asprintf(&ir_expr->left->value->u.literal, "%d", n->i);
  return res;
}

int ipts_convert_match_option(struct match_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting match_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("match");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_helper_option(struct helper_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting helper_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("helper");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_icmp_type_option(struct icmp_type_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting icmp_type_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("icmp_type");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_reject_with_option(struct reject_with_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting reject_with_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("reject_with");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_uid_owner_option(struct uid_owner_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting uid_owner_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("uid_owner");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_log_prefix_option(struct log_prefix_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting log_prefix_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("log_prefix");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_limit_option(struct limit_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting limit_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("limit");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_state_option(struct state_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting state_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("state");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_to_source_option(struct to_source_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting to_source_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("to_source");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_to_ports_option(struct to_ports_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting to_ports_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("to_ports");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_protocol_option(struct protocol_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting protocol_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("protocol");
  if (n->identifier) {
    ir_expr->left->value = ir_value_new();
    res = ipts_convert_identifier(n->identifier, ir_expr->left->value);
  }
  return res;
}

int ipts_convert_dport_option(struct dport_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting dport_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("dport");
  if (n->not_identifier) {
    res = ipts_convert_not_identifier(n->not_identifier, ir_expr->left);
  } else if (n->not_range) {
      res = ipts_convert_not_range(n->not_range, ir_expr->left);
  }
  return res;
}

int ipts_convert_sport_option(struct sport_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting sport_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("sport");
  if (n->not_identifier) {
    res = ipts_convert_not_identifier(n->not_identifier, ir_expr->left);
  } else if (n->not_range) {
      res = ipts_convert_not_range(n->not_range, ir_expr->left);
  }
  return res;
}

int ipts_convert_out_interface_option(struct out_interface_option_s * n, struct ir_expr_s * ir_expr) {
  int res = 1;
  eprint("converting out_interface_option\n");
  assert(ir_expr);
  assert(ir_expr->value);
  assert(ir_expr->left);
  ir_expr->value->u.predicate = strdup("out_interface");
  if (n->not_identifier) {
    res = ipts_convert_not_identifier(n->not_identifier, ir_expr->left);
  }
  return res;
}
#endif

int ipts_convert_in_interface_option(struct option_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;

    eprint("converting in_interface_option\n");

    assert(ir_expr);
    assert(ir_expr->left);

    ir_expr->value->u.predicate = strdup("in_interface");

    if (n->not_identifier) {
	res = ipts_convert_not_identifier(n->not_identifier, ir_expr->left);
    }

    return res;
}

#if 0
int ipts_convert_source_option(struct source_option_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;

    eprint("converting source_option\n");

    assert(ir_expr);
    assert(ir_expr->left);

    ir_expr->value->u.predicate = strdup("source");

    if (n->not_identifier) {
	ipts_convert_not_identifier(n->not_identifier, ir_expr->left);
    }

    return res;
}

int ipts_convert_destination_option(struct destination_option_s * n, struct ir_expr_s * ir_expr) {
    int res = 1;

    eprint("converting destination_option\n");

    assert(ir_expr);
    assert(ir_expr->value);
    assert(ir_expr->left);

    ir_expr->value->u.predicate = strdup("destination");

    if (n->not_identifier) {
	ipts_convert_not_identifier(n->not_identifier, ir_expr->left);
    }

    return res;
}
#endif

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
 * convert an ipts option into an internal representation predicate
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
	  res = ipts_convert_in_interface_option(n, e);
	  break;
	default:
	  fprintf(stderr, "warning: unknown option type %d\n", n->type);
      }
      #if 0
      } else if (n->source_option) {
	  eprint("going to convert source option\n");
	  res = ipts_convert_source_option(n->source_option, e);
      } else if (n->destination_option) {
	  eprint("going to convert destination option\n");
	  res = ipts_convert_destination_option(n->destination_option, e);
      } else if (n->protocol_option) {
	  eprint("going to convert protocol option\n");
	  res = ipts_convert_protocol_option(n->protocol_option, e);
      } else if (n->match_option) {
	  eprint("going to convert match option\n");
	  res = ipts_convert_match_option(n->match_option, e);
      } else if (n->dport_option) {
	  eprint("going to convert dport option\n");
	  res = ipts_convert_dport_option(n->dport_option, e);
      } else if (n->sport_option) {
	  eprint("going to convert sport option\n");
	  res = ipts_convert_sport_option(n->sport_option, e);
      } else if (n->to_ports_option) {
	  eprint("going to convert to_ports option\n");
	  res = ipts_convert_to_ports_option(n->to_ports_option, e);
      } else if (n->out_interface_option) {
	  eprint("going to convert out_interface option\n");
	  res = ipts_convert_out_interface_option(n->out_interface_option, e);
      } else if (n->to_source_option) {
	  eprint("going to convert to_source option\n");
	  res = ipts_convert_to_source_option(n->to_source_option, e);
      } else if (n->state_option) {
	  eprint("going to convert state option\n");
	  res = ipts_convert_state_option(n->state_option, e);
      } else if (n->limit_option) {
	  eprint("going to convert limit option\n");
	  res = ipts_convert_limit_option(n->limit_option, e);
      } else if (n->log_prefix_option) {
	  eprint("going to convert log_prefix option\n");
	  res = ipts_convert_log_prefix_option(n->log_prefix_option, e);
      } else if (n->uid_owner_option) {
	  eprint("going to convert uid_owner option\n");
	  res = ipts_convert_uid_owner_option(n->uid_owner_option, e);
      } else if (n->tcp_flags_option) {
	  eprint("going to convert tcp_flags option\n");
	  res = ipts_convert_tcp_flags_option(n->tcp_flags_option, e);
      } else if (n->reject_with_option) {
	  eprint("going to convert reject_with option\n");
	  res = ipts_convert_reject_with_option(n->reject_with_option, e);
      } else if (n->icmp_type_option) {
	  eprint("going to convert icmp_type option\n");
	  res = ipts_convert_icmp_type_option(n->icmp_type_option, e);
      } else if (n->fragment_option) {
	  eprint("going to convert fragment option\n");
	  res = ipts_convert_fragment_option(n->fragment_option, e);
      } else if (n->clamp_mss_to_pmtu_option) {
	  eprint("going to convert clamp_mss_to_pmtu option\n");
	  res = ipts_convert_clamp_mss_to_pmtu_option(n->clamp_mss_to_pmtu_option, e);
      } else if (n->helper_option) {
	  eprint("going to convert helper option\n");
	  res = ipts_convert_helper_option(n->helper_option, e);
      } else if (n->syn_option) {
	  eprint("going to convert syn option\n");
	  res = ipts_convert_syn_option(n->syn_option, e);
      }
      #endif
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

int ipts_convert_rule_list(struct rule_list_s * n, struct ir_rule_s * ir_rule) {
    int res = 1;

    eprint("converting rule list\n");

    assert(ir_rule);

    
    if (n->list) {
      struct ir_rule_s * r, * i;
      r = ir_rule_new();
      res = ipts_convert_rule(n->rule, r);
      res = ipts_convert_rule_list(n->list, ir_rule);

      /* find the end of the ir_rule chain to place the next rule */
      for (i = ir_rule; i->next != NULL; i = i->next);
      i->next = r;

    } else {
      res = ipts_convert_rule(n->rule, ir_rule);
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
