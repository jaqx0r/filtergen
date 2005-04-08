/* internal representation of packet filter
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

enum ir_operator { IR_AND, IR_OR };

/** binary expression tree */
struct ir_node_s {
    struct ir_node_s * left;
    enum ir_operator op;
    struct ir_node_s * right;
};

enum ir_action_type { IR_ACCEPT, IR_DROP, IR_REJECT };

/** declares action to take with packet matching associated predicates */
struct ir_action_s {
    /** action to take */
    enum ir_action_type action;
    /** options that affect action taken
     * e.g. --reject-with icmp-host-unreachable */
    char * options;
};

struct ir_rule_s {
    struct ir_node_s * predicates;
    struct ir_action_s * action;
};

/** top level container for the internal representation.  Names reflect
 * those used in iptables, but have analogies with other packet filters.
 */
struct ir_s {
    /** list of rules for the main packet filter */
    struct ir_rule_s ** filter;
    /** list of rules for Network Address Translation */
    struct ir_rule_s ** nat;
    /** list of rules for packet modification, c.f. pf's "options: scrub" */
    struct ir_rule_s ** mangle;
    /* TODO: add "conf" section, correlating to pf's "options: set",
       and for modifying kernel sysctl parameters */
};
    
    
#endif /* __FILTERGEN_IR_H__ */
