/* ast emitter
 * output should be identical (syntactically) to the input
 */

#include <stdio.h>
#include <stdlib.h>
#include "../ast.h"
#include "../parser.h"

int yyparse(void *);
int emittrace = 0;

#define EMIT(x) void emit_##x(struct x##_s * n)

#define eprint(x) if (emittrace) printf(x)

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
    switch (n->type) {
      case TOK_LOCAL:
	printf("local"); break;
      case TOK_FORWARD:
	printf("forward"); break;
      default:
	printf("error"); break;
    }
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
    if (n->list) {
	emit_icmptype_argument_list(n->list);
    }
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
    printf("%s", n->proto);
}

EMIT(protocol_argument_list) {
    if (n->list) {
	eprint("emitting protocol_argument_list\n");
	emit_protocol_argument_list(n->list);
	printf(" ");
    }
    eprint("emitting simple_protocol_argument\n");
    emit_simple_protocol_argument(n->arg);
}

EMIT(compound_protocol_argument) {
    printf("{ ");
    if (n->list) {
	eprint("emitting protocol_argument_list\n");
	emit_protocol_argument_list(n->list);
    }
    printf(" }");
}

EMIT(protocol_argument) {
    if (n->compound) {
	eprint("emitting compound_protocol_argument\n");
	emit_compound_protocol_argument(n->compound);
    } else {
	eprint("emitting simple_protocol_argument\n");
	emit_simple_protocol_argument(n->simple);
    }
}

EMIT(protocol_specifier) {
    printf("proto ");
    eprint("emitting protocol_argument\n");
    emit_protocol_argument(n->arg);
}

EMIT(port_single) {
    if (n->name) {
	eprint("emitting port name\n");
	printf("%s", n->name);
    } else {
	eprint("emitting port number\n");
	printf("%d", n->num);
    }
}

EMIT(port_range) {
    eprint("emitting port_single\n");
    emit_port_single(n->min);
    printf(":");
    eprint("emitting port_single\n");
    emit_port_single(n->max);
}

EMIT(simple_port_argument) {
    if (n->range) {
	eprint("emitting port_range\n");
	emit_port_range(n->range);
    } else if (n->single) {
	eprint("emitting port_single\n");
	emit_port_single(n->single);
    }
}

EMIT(port_argument_list) {
    if (n->list) {
	eprint("emitting port_argument_list\n");
	emit_port_argument_list(n->list);
	printf(" ");
    }
    eprint("emitting simple_port_argument\n");
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
    if (n->type == TOK_DPORT) {
	printf("dport ");
    } else if (n->type == TOK_SPORT) {
	printf("sport ");
    }
    emit_port_argument(n->arg);
}

EMIT(host_part) {
    if (n->host) {
	printf("%s", n->host);
    } else {
	printf("%d.%d.%d.%d", n->ip1, n->ip2, n->ip3, n->ip4);
    }
}

EMIT(netmask_part) {
    printf("%d.%d.%d.%d", n->mask1, n->mask2, n->mask3, n->mask4);
}

EMIT(simple_host_argument) {
    if (n->host) {
	eprint("emitting host_part\n");
	emit_host_part(n->host);
    }
    if (n->netmask) {
	printf("/");
	eprint("emitting netmask_part\n");
	emit_netmask_part(n->netmask);
    }
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
    if (n->type == TOK_SOURCE) {
	printf("source ");
    } else if (n->type == TOK_DEST) {
	printf("dest ");
    }
    emit_host_argument(n->arg);
}

EMIT(target_specifier) {
    switch (n->type) {
      case TOK_ACCEPT:
	printf("accept"); break;
      case TOK_REJECT:
	printf("reject"); break;
      case TOK_DROP:
	printf("drop"); break;
      case TOK_REDIRECT:
	printf("redirect"); break;
      case TOK_MASQ:
	printf("masq"); break;
      case TOK_LOG:
	printf("log");
	if (n->logtext) {
	    printf(" text \"%s\"", n->logtext);
	}
	break;
      default:
	printf("error"); break;
    }
}

EMIT(simple_direction_argument) {
    eprint("emitting identifier\n");
    printf("%s", n->identifier);
}

EMIT(direction_argument_list) {
    if (n->list) {
	eprint("emitting direction_argument_list\n");
	emit_direction_argument_list(n->list);
	printf(" ");
    }
    eprint("emitting simple_direction_argument\n");
    emit_simple_direction_argument(n->arg);
}

EMIT(compound_direction_argument) {
    printf("{ ");
    if (n->list) {
	eprint("emitting direction_argument_list\n");
	emit_direction_argument_list(n->list);
    }
    printf(" }");
}

EMIT(direction_argument) {
    if (n->compound) {
	eprint("emitting compound_direction_argument\n");
	emit_compound_direction_argument(n->compound);
    } else {
	eprint("emitting simple_direction_argument\n");
	emit_simple_direction_argument(n->simple);
    }
}

EMIT(direction_specifier) {
    if (n->type == TOK_INPUT) {
	printf("input ");
    } else {
	printf("output ");
    }
    eprint("emitting direction_argument\n");
    emit_direction_argument(n->arg);
}

EMIT(specifier) {
    if (n->compound) {
	eprint("emitting compound_specifier\n");
	emit_compound_specifier(n->compound);
    } else if (n->direction) {
	eprint("emitting direction_specifier\n");
	emit_direction_specifier(n->direction);
    } else if (n->target) {
	eprint("emitting target_specifier\n");
	emit_target_specifier(n->target);
    } else if (n->host) {
	eprint("emitting host_specifier\n");
	emit_host_specifier(n->host);
    } else if (n->port) {
	eprint("emitting port_specifier\n");
	emit_port_specifier(n->port);
    } else if (n->protocol) {
	eprint("emitting protocol_specifier\n");
	emit_protocol_specifier(n->protocol);
    } else if (n->icmptype) {
	eprint("emitting icmptype_specifier\n");
	emit_icmptype_specifier(n->icmptype);
    } else if (n->routing) {
	eprint("emitting routing_specifier\n");
	emit_routing_specifier(n->routing);
    } else {
	eprint("emitting chaingroup_specifier\n");
	emit_chaingroup_specifier(n->chaingroup);
    }
}

EMIT(negated_specifier) {
    if (n->negated) {
	printf("! ");
    }
    eprint("emitting specifier\n");
    emit_specifier(n->spec);
}

EMIT(specifier_list) {
    if (n->list) {
	eprint("emitting specifier_list\n");
	emit_specifier_list(n->list);
	/* specifiers are separated by spaces */
	printf(" ");
    }
    eprint("emitting negated_specifier\n");
    emit_negated_specifier(n->spec);
}

EMIT(rule) {
    if (n->list) {
	eprint("emitting specifier_list\n");
	emit_specifier_list(n->list);
    }
    /* rules end in a semicolon and newline */
    printf(";\n");
}

EMIT(rule_list) {
    if (n->list) {
	eprint("emitting rule_list\n");
	emit_rule_list(n->list);
    }
    if (n->rule) {
	eprint("emitting rule\n");
	emit_rule(n->rule);
    }
}

EMIT(ast) {
    if (n->list) {
	eprint("emitting rule_list\n");
	emit_rule_list(n->list);
    }
}

int main(int argc, char ** argv) {
    char * EMITTRACE;
    struct ast_s ast;
    int res;

    EMITTRACE = getenv("EMITTRACE");
    emittrace = EMITTRACE ? atoi(EMITTRACE) : 0;

    res = yyparse((void *)&ast);

    if (res != 0) {
	printf("yyparse returned %d\n", res);
	return 1;
    }

    eprint("emitting ast\n");
    emit_ast(&ast);

    return 0;
}
