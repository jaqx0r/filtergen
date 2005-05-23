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

enum ir_expr_type { IR_EXPR_NONE, IR_EXPR_VALUE, IR_EXPR_UNARY, IR_EXPR_BINARY };

enum ir_operator { IR_OP_NONE, IR_OP_AND, IR_OP_OR, IR_OP_NOT, IR_OP_PRED };

/** expression tree */
struct ir_expr_s {
  /** type of this expression node */
  enum ir_expr_type type;

  /** the value of this expression node */
  char * value;

  /** the type of operator for this expression */
  enum ir_operator operator;

  /** the left child, or first argument to this expression */
  struct ir_expr_s * left;

  /** the right child, or second argument, to this expression */
  struct ir_expr_s * right;
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
    char * options;
};

/** Represents a rule in a packet filter.
 * If a packet satisfies all the predicates, then the associated action
 * will be taken on that packet. */
struct ir_rule_s {
    struct ir_expr_s * expr;
    struct ir_action_s * action;

  struct ir_rule_s * next;
};

/** Top level container for the internal representation.  The names reflect
 * those used in iptables, but have analogies with other packet filters.
 */
struct ir_s {
    /** list of rules for the main packet filter */
    struct ir_rule_s * filter;
    /** list of rules for Network Address Translation */
    struct ir_rule_s * nat;
    /** list of rules for packet modification.
	c.f. pf's "options: scrub" */
    struct ir_rule_s * mangle;
    /* TODO: add "conf" section, correlating to pf's "options: set",
       and for modifying kernel sysctl parameters */
};

#define IR_NEW(x) struct x##_s * x##_new()
#define IR_FREE(x) void x##_free(struct x##_s *)

IR_NEW(ir);
IR_NEW(ir_rule);
IR_NEW(ir_action);
IR_NEW(ir_expr);
IR_FREE(ir);
IR_FREE(ir_rule);
IR_FREE(ir_action);
IR_FREE(ir_expr);

#undef IR_NEW
#undef IR_FREE

#endif /* __FILTERGEN_IR_H__ */
