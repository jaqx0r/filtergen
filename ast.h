/* syntax tree data structure definitions
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

#ifndef __AST_H__
#define __AST_H__

struct specifier_list_s;

struct subrule_list_s {
    struct subrule_list_s * subrule_list;
    struct specifier_list_s * specifier_list;
};

struct chaingroup_specifier_s {
    char * name;
    struct subrule_list_s * list;
};

struct compound_specifier_s {
    struct subrule_list_s * list;
};

struct option_specifier_s {
    int type;
    char * logmsg;
};

struct icmptype_argument_s {
    char * icmptype;
};

struct icmptype_argument_list_s {
    struct icmptype_argument_list_s * list;
    struct icmptype_argument_s * arg;
};

struct icmptype_specifier_s {
    struct icmptype_argument_list_s * list;
};

struct protocol_argument_s {
    char * proto;
};

struct protocol_argument_list_s {
    struct protocol_argument_list_s * list;
    struct protocol_argument_s * arg;
};

struct protocol_specifier_s {
    struct protocol_argument_list_s * list;
};

struct port_argument_s {
    char * port_min;
    char * port_max;
};

struct port_argument_list_s {
    struct port_argument_list_s * list;
    struct port_argument_s * arg;
};

struct port_specifier_s {
    int type;
    struct port_argument_list_s * list;
};

struct host_argument_s {
    char * host;
    char * mask;
};

struct host_argument_list_s {
    struct host_argument_list_s * list;
    struct host_argument_s * arg;
};

struct host_specifier_s {
    int type;
    struct host_argument_list_s * list;
};

struct target_specifier_s {
    int type;
};

struct direction_argument_s {
    char * direction;
};

struct direction_argument_list_s {
    struct direction_argument_list_s * list;
    struct direction_argument_s * arg;
};

struct direction_specifier_s {
    int type;
    struct direction_argument_list_s * list;
};

struct specifier_s {
    struct compound_specifier_s * compound;
    struct direction_specifier_s * direction;
    struct target_specifier_s * target;
    struct host_specifier_s * host;
    struct port_specifier_s * port;
    struct protocol_specifier_s * protocol;
    struct icmptype_specifier_s * icmptype;
    struct option_specifier_s * option;
    struct chaingroup_specifier_s * chaingroup;
};

struct negated_specifier_s {
    int negated;
    struct specifier_s * spec;
};

struct specifier_list_s {
    struct specifier_list_s * list;
    struct negated_specifier_s * spec;
};

struct rule_s {
    struct specifier_list_s * list;
};

struct rule_list_s {
    struct rule_list_s * list;
    struct rule_s * rule;
};

struct ast_s {
    struct rule_list_s * list;
};

#endif /* __AST_H__ */
