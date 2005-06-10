/* internal representation of packet filter description
 *
 * Copyright (c) 2005 Jamie Wilkinson <jaq@spacepants.org>
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

#ifndef __FILTERGEN_IR_H__
#define __FILTERGEN_IR_H__

enum ir_value_type { IR_VAL_OPERATOR, IR_VAL_PREDICATE, IR_VAL_LITERAL, IR_VAL_RANGE };

enum ir_operator { IR_OP_NONE, IR_OP_AND, IR_OP_OR, IR_OP_NOT, IR_OP_PRED };

struct ir_value_s {
    /** type of this value */
    enum ir_value_type type;

    union {
	/** operator */
	enum ir_operator operator;
	/** name of the predicate */
	char * predicate;
	/** value of the literal */
	char * literal;
    } u;
};
    
/** expression tree */
struct ir_expr_s {
    /** the left child, or first argument to this expression */
    struct ir_expr_s * left;
    /** the right child, or second argument, to this expression */
    struct ir_expr_s * right;

    /** The keystone of this expression tree */
    struct ir_value_s * value;
};

/** Actions that can be performed by packet filters. */
enum ir_action_type { IR_ACCEPT, IR_DROP, IR_REJECT, IR_LOG };

/** Declares action to take on packets. */
struct ir_action_s {
    /** Type of action to take on this packet. */
    enum ir_action_type type;
    /** Options to modify the behaviour of the action.
     * e.g. --reject-with icmp-host-unreachable .
     * TODO: change the char * into some enumeration to make it pf agnostic
     */
  struct ir_expr_s * option;
};

/** Represents a rule in a packet filter.
 * If a packet satisfies all the predicates, then the associated action
 * will be taken on that packet. */
struct ir_rule_s {
    struct ir_expr_s * expr;
    struct ir_action_s * action;

    struct ir_rule_s * next;
};

/** Represents a rule chain, which is a named block of rules that can be
 *  called by other blocks as the target. */
struct ir_chain_s {
    /** The name of the chain. */
    char * name;
    /** The list of rules in this chain. */
    struct ir_rule_s * rule;

    /** The next chain in the chain chain. */
    struct ir_chain_s * next;
};

/** Top level container for the internal representation.  The names reflect
 * those used in iptables, but have analogies with other packet filters.
 */
struct ir_s {
    /** list of rules for the main packet filter */
    struct ir_chain_s * filter;
    /** list of rules for Network Address Translation */
    struct ir_chain_s * nat;
    /** list of rules for packet modification.
	c.f. pf's "options: scrub" */
    struct ir_chain_s * mangle;
    /* TODO: add "conf" section, correlating to pf's "options: set",
       and for modifying kernel sysctl parameters */
};

#define IR_NEW(x) struct x##_s * x##_new()
#define IR_FREE(x) void x##_free(struct x##_s *)

IR_NEW(ir);
IR_NEW(ir_chain);
IR_NEW(ir_rule);
IR_NEW(ir_action);
IR_NEW(ir_expr);
IR_NEW(ir_value);
IR_FREE(ir);
IR_FREE(ir_chain);
IR_FREE(ir_rule);
IR_FREE(ir_action);
IR_FREE(ir_expr);
IR_FREE(ir_value);

#undef IR_NEW
#undef IR_FREE

struct ir_expr_s * ir_expr_new_operator(enum ir_operator);
struct ir_expr_s * ir_expr_new_predicate(const char *);
struct ir_expr_s * ir_expr_new_literal(const char *);
struct ir_expr_s * ir_expr_new_range();

#endif /* __FILTERGEN_IR_H__ */
