/* syntax tree
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
    struct subrule_list_s * subrule_list;
};

struct log_text_argument_s {
    char * identifier;
};

struct compound_specifier_s {
    struct subrule_list_s * subrule_list;
};

struct routing_specifier_s {
    /* type */
};

struct simple_icmptype_argument_s {
    char * identifier;
};

struct icmptype_argument_list_s {
    struct icmptype_argument_list_s * icmptype_argument_list;
    struct simple_icmptype_argument_s * simple_icmptype_argument;
};

struct compound_icmptype_argument_s {
    struct icmptype_argument_list_s * icmptype_argument_list;
};

struct icmptype_argument_s {
    struct compound_icmptype_argument_s * compound_icmptype_argument;
    struct simple_icmptype_argument_s * simple_icmptype_argument;
};

struct icmptype_specifier_s {
    struct icmptype_argument_s * icmptype_argument;
};

struct simple_protocol_argument_s {
    char * identifier;
};

struct protocol_argument_list_s {
    struct protocol_argument_list_s * protocol_argument_list;
    struct simple_protocol_argument_s * simple_protocol_argument;
};

struct compound_protocol_argument_s {
    struct protocol_argument_list_s * protocol_argument_list;
};

struct protocol_argument_s {
    struct compound_protocol_argument_s * compound_protocol_argument;
    struct simple_protocol_argument_s * simple_protocol_argument;
};

struct protocol_specifier_s {
    struct protocol_argument_s * protocol_argument;
};

struct simple_port_argument_s {
    int port_min;
    int port_max;
};

struct port_argument_list_s {
    struct port_argument_list_s * port_argument_list;
    struct simple_port_argument_s * simple_port_argument;
};

struct compound_port_argument_s {
    struct port_argument_list_s * port_argument_list;
};

struct port_argument_s {
    struct compound_port_argument_s * compound_port_argument;
    struct simple_port_argument_s * simple_port_argument;
};

struct port_specifier_s {
    /* type */
    struct port_argument_s * port_argument;
};

struct simple_host_argument_s {
    char * host;
};

struct host_argument_list_s {
    struct host_argument_list_s * host_argument_list;
    struct simple_host_argument_s * simple_host_argument;
};

struct compound_host_argument_s {
    struct host_argument_list_s * host_argument_list;
};

struct host_argument_s {
    struct compound_host_argument_s * compound_host_argument;
    struct simple_host_argument_s * simple_host_argument;
};

struct host_specifier_s {
    /* type */
    struct host_argument_s * host_argument;
};

struct log_target_specifier_s {
    char * text;
};

struct target_specifier_s {
    /* type */
    struct log_target_specifier_s * log_target_specifier;
};

struct simple_direction_argument_s {
    char * identifier;
};

struct direction_argument_list_s {
    struct direction_argument_list_s * direction_argument_list;
    struct simple_direction_argument_s * simple_direction_argument;
};

struct compound_direction_argument_s {
    struct direction_argument_list_s * direction_argument_list;
};

struct direction_argument_s {
    struct compound_direction_argument_s * compound_direction_argument;
    struct simple_direction_argument_s * simple_direction_argument;
};

struct direction_specifier_s {
    /* type */
    struct direction_argument_s * direction_argument;
};

struct specifier_s {
    struct compound_specifier_s * compound_specifier;
    struct direction_specifier_s * direction_specifier;
    struct target_specifier_s * target_specifier;
    struct host_specifier_s * host_specifier;
    struct port_specifier_s * port_specifier;
    struct protocol_specifier_s * protocol_specifier;
    struct icmptype_specifier_s * icmptype_specifier;
    struct routing_specifier_s * routing_specifier;
    struct chaingroup_specifier_s * chaingroup_specifier;
};

struct negated_specifier {
    /* type */
    struct specifier_ * specifier;
};

struct specifier_list_s {
    struct specifier_list_s * specifier_list;
    struct negated_specifier_s * negated_specifier;
};

struct rule_s {
    struct specifier_list_s * specifier_list;
};

struct rule_list_s {
    struct rule_list_s * rule_list;
    struct rule_s * rule;
};

struct ast_s {
    struct rule_list_s * rule_list;
};

#endif /* __AST_H__ */
