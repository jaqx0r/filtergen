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
    struct subrule_list_s * list;
};

struct compound_specifier_s {
    struct subrule_list_s * list;
};

struct routing_specifier_s {
    /* type */
};

struct simple_icmptype_argument_s {
    char * identifier;
};

struct icmptype_argument_list_s {
    struct icmptype_argument_list_s * list;
    struct simple_icmptype_argument_s * arg;
};

struct compound_icmptype_argument_s {
    struct icmptype_argument_list_s * list;
};

struct icmptype_argument_s {
    struct compound_icmptype_argument_s * compound;
    struct simple_icmptype_argument_s * simple;
};

struct icmptype_specifier_s {
    struct icmptype_argument_s * arg;
};

struct simple_protocol_argument_s {
    char * identifier;
};

struct protocol_argument_list_s {
    struct protocol_argument_list_s * list;
    struct simple_protocol_argument_s * arg;
};

struct compound_protocol_argument_s {
    struct protocol_argument_list_s * list;
};

struct protocol_argument_s {
    struct compound_protocol_argument_s * compound;
    struct simple_protocol_argument_s * simple;
};

struct protocol_specifier_s {
    struct protocol_argument_s * arg;
};

struct simple_port_argument_s {
    int port_min;
    int port_max;
};

struct port_argument_list_s {
    struct port_argument_list_s * list;
    struct simple_port_argument_s * arg;
};

struct compound_port_argument_s {
    struct port_argument_list_s * list;
};

struct port_argument_s {
    struct compound_port_argument_s * compound;
    struct simple_port_argument_s * simple;
};

struct port_specifier_s {
    /* type */
    struct port_argument_s * arg;
};

struct simple_host_argument_s {
    char * host;
};

struct host_argument_list_s {
    struct host_argument_list_s * list;
    struct simple_host_argument_s * arg;
};

struct compound_host_argument_s {
    struct host_argument_list_s * list;
};

struct host_argument_s {
    struct compound_host_argument_s * compound;
    struct simple_host_argument_s * simple;
};

struct host_specifier_s {
    /* type */
    struct host_argument_s * arg;
};

struct log_text_argument_s {
    char * text;
};

struct log_target_specifier_s {
    struct log_text_argument_s * arg;
};

struct target_specifier_s {
    int type;
    struct log_target_specifier_s * log;
};

struct simple_direction_argument_s {
    char * identifier;
};

struct direction_argument_list_s {
    struct direction_argument_list_s * list;
    struct simple_direction_argument_s * arg;
};

struct compound_direction_argument_s {
    struct direction_argument_list_s * list;
};

struct direction_argument_s {
    struct compound_direction_argument_s * compound;
    struct simple_direction_argument_s * simple;
};

struct direction_specifier_s {
    int type;
    struct direction_argument_s * arg;
};

struct specifier_s {
    struct compound_specifier_s * compound;
    struct direction_specifier_s * direction;
    struct target_specifier_s * target;
    struct host_specifier_s * host;
    struct port_specifier_s * port;
    struct protocol_specifier_s * protocol;
    struct icmptype_specifier_s * icmptype;
    struct routing_specifier_s * routing;
    struct chaingroup_specifier_s * chaingroup;
};

struct negated_specifier_s {
    /* type */
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
