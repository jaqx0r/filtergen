/* ast emitter
 * output should be identical (syntactically) to the input
 */

#include <stdio.h>
#include "../ast.h"

int yyparse(void *);

#define EMIT(x) void emit_##x(struct x##_s * n)

EMIT(specifier_list);

EMIT(subrule_list) {
    if (n->subrule_list) {
	emit_subrule_list(n->subrule_list);
	printf(";\n");
    }
    emit_specifier_list(n->specifier_list);
}

EMIT(chaingroup_specifier) {
    printf("[ ");
    if (n->name)
	printf("\"%s\" ", n->name);
    printf("\n");
    emit_subrule_list(n->list);
    printf("]\n");
}

EMIT(compound_specifier) {
    printf("{\n");
    emit_subrule_list(n->list);
    printf("}\n");
}

EMIT(routing_specifier) {
    /* print type */
}

EMIT(simple_icmptype_argument) {
    printf("%s", n->identifier);
}

EMIT(icmptype_argument_list) {
    if (n->list) {
	emit_icmptype_argument_list(n->list);
	printf(" ");
    }
    emit_simple_icmptype_argument(n->arg);
}

EMIT(compound_icmptype_argument) {
    printf("{ ");
    emit_icmptype_argument_list(n->list);
    printf(" }");
};

EMIT(icmptype_argument) {
    if (n->compound)
	emit_compound_icmptype_argument(n->compound);
    else
	emit_simple_icmptype_argument(n->simple);
}

EMIT(icmptype_specifier) {
    printf("icmptype ");
    emit_icmptype_argument(n->arg);
}

EMIT(simple_protocol_argument) {
    printf("%s ", n->identifier);
}

EMIT(protocol_argument_list) {
    if (n->list) {
	emit_protocol_argument_list(n->list);
	printf(" ");
    }
    emit_simple_protocol_argument(n->arg);
}

EMIT(compound_protocol_argument) {
    printf("{ ");
    emit_protocol_argument_list(n->list);
    printf(" }");
}

EMIT(protocol_argument) {
    if (n->compound)
	emit_compound_protocol_argument(n->compound);
    else
	emit_simple_protocol_argument(n->simple);
}

EMIT(protocol_specifier) {
    /* print type */
    emit_protocol_argument(n->arg);
}

EMIT(simple_port_argument) {
    printf("%d", n->port_min);
    /* FIX this */
    printf(":%d", n->port_max);
}

EMIT(port_argument_list) {
    if (n->list)
	emit_port_argument_list(n->list);
    emit_simple_port_argument(n->arg);
}

EMIT(compound_port_argument) {
    printf("{ ");
    emit_port_argument_list(n->list);
    printf(" }");
}

EMIT(port_argument) {
    if (n->compound)
	emit_compound_port_argument(n->compound);
    else
	emit_simple_port_argument(n->simple);
}

EMIT(port_specifier) {
    /* print type */
    emit_port_argument(n->arg);
}

EMIT(simple_host_argument) {
    printf("%s", n->host);
}

EMIT(host_argument_list) {
    if (n->list) {
	emit_host_argument_list(n->list);
	printf(" ");
    }
    emit_simple_host_argument(n->arg);
}

EMIT(compound_host_argument) {
    printf("{ ");
    emit_host_argument_list(n->list);
    printf(" }");
}

EMIT(host_argument) {
    if (n->compound)
	emit_compound_host_argument(n->compound);
    else
	emit_simple_host_argument(n->simple);
}

EMIT(host_specifier) {
    /* print type */
    emit_host_argument(n->arg);
}

EMIT(log_text_argument) {
    printf("text \"%s\" ", n->text);
}

EMIT(log_target_specifier) {
    printf("log ");
    if (n->arg)
	emit_log_text_argument(n->arg);
}

EMIT(target_specifier) {
    /* print type */
}

EMIT(simple_direction_argument) {
    printf("%s ", n->identifier);
}

EMIT(direction_argument_list) {
    if (n->list) {
	emit_direction_argument_list(n->list);
	printf(" ");
    }
    emit_simple_direction_argument(n->arg);
}

EMIT(compound_direction_argument) {
    printf("{ ");
    emit_direction_argument_list(n->list);
    printf(" }");
}

EMIT(direction_argument) {
    if (n->compound)
	emit_compound_direction_argument(n->compound);
    else
	emit_simple_direction_argument(n->simple);
}

EMIT(direction_specifier) {
    /* print type */
    emit_direction_argument(n->arg);
}

EMIT(specifier) {
    if (n->compound)
	emit_compound_specifier(n->compound);
    else if (n->direction)
	emit_direction_specifier(n->direction);
    else if (n->target)
	emit_target_specifier(n->target);
    else if (n->host)
	emit_host_specifier(n->host);
    else if (n->port)
	emit_port_specifier(n->port);
    else if (n->protocol)
	emit_protocol_specifier(n->protocol);
    else if (n->icmptype)
	emit_icmptype_specifier(n->icmptype);
    else if (n->routing)
	emit_routing_specifier(n->routing);
    else
	emit_chaingroup_specifier(n->chaingroup);
}

EMIT(negated_specifier) {
    /* print type */
    emit_specifier(n->spec);
}

EMIT(specifier_list) {
    if (n->list) {
	emit_specifier_list(n->list);
	/* specifiers are separated by spaces */
	printf(" ");
    }
    emit_negated_specifier(n->spec);
}

EMIT(rule) {
    if (n->list)
	emit_specifier_list(n->list);
    /* rules end in a semicolon and newline */
    printf(";\n");
}

EMIT(rule_list) {
    if (n->list) {
	emit_rule_list(n->list);
    }
    if (n->rule) {
	emit_rule(n->rule);
    }
}

EMIT(ast) {
    if (n->list)
	emit_rule_list(n->list);
}

int main(int argc, char ** argv) {
    struct ast_s ast;
    int res;

    res = yyparse((void *)&ast);

    if (res != 0) {
	printf("yyparse returned %d\n", res);
	return 1;
    }

    emit_ast(&ast);

    return 0;
}
