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

#define eprint(x) if (emittrace) fprintf(stderr, x)

EMIT(specifier_list);

EMIT(subrule_list) {
    if (n->subrule_list) {
	eprint("emitting subrule_list\n");
	emit_subrule_list(n->subrule_list);
	printf(";\n");
    }
    if (n->specifier_list) {
	eprint("emitting specifier_list\n");
	emit_specifier_list(n->specifier_list);
    }
}

EMIT(chaingroup_specifier) {
    printf("[");
    if (n->name) {
	printf(" \"%s\"", n->name);
    }
    printf("\n");
    eprint("emitting subrule_list\n");
    emit_subrule_list(n->list);
    printf("]");
}

EMIT(compound_specifier) {
    printf("{");
    if (n->list) {
	eprint("emitting subrule_list\n");
	emit_subrule_list(n->list);
    }
    printf("}");
}

EMIT(option_specifier) {
    switch (n->type) {
      case TOK_LOCAL:
	printf("local"); break;
      case TOK_FORWARD:
	printf("forward"); break;
      case TOK_ONEWAY:
	printf("oneway"); break;
      case TOK_LOG:
	if (n->logmsg) {
	  printf("log text \"%s\"", n->logmsg);
	} else {
	  printf("log");
	}
	break;
      default:
	printf("error"); break;
    }
}

EMIT(icmptype_argument) {
    printf("%s", n->icmptype);
}

EMIT(icmptype_argument_list) {
    if (n->list) {
	emit_icmptype_argument_list(n->list);
	printf(" ");
    }
    emit_icmptype_argument(n->arg);
}

EMIT(icmptype_specifier) {
    printf("icmptype { ");
    if (n->list) 
      emit_icmptype_argument_list(n->list);
    printf(" }");
}

EMIT(protocol_argument) {
    printf("%s", n->proto);
}

EMIT(protocol_argument_list) {
    if (n->list) {
	eprint("emitting protocol_argument_list\n");
	emit_protocol_argument_list(n->list);
	printf(" ");
    }
    eprint("emitting protocol_argument\n");
    emit_protocol_argument(n->arg);
}

EMIT(protocol_specifier) {
    printf("proto { ");
    if (n->list) {
      eprint("emitting protocol_argument\n");
      emit_protocol_argument_list(n->list);
    }
    printf(" }");
}

EMIT(port_argument) {
    if (n->port_min) {
	printf("%s", n->port_min);
    }
    if (n->port_max) {
	printf(":%s", n->port_max);
    }
}

EMIT(port_argument_list) {
    if (n->list) {
	eprint("emitting port_argument_list\n");
	emit_port_argument_list(n->list);
	printf(" ");
    }
    eprint("emitting port_argument\n");
    emit_port_argument(n->arg);
}

EMIT(port_specifier) {
    if (n->type == TOK_DPORT) {
	printf("dport ");
    } else if (n->type == TOK_SPORT) {
	printf("sport ");
    }
    printf("{ ");
    if (n->list)
      emit_port_argument_list(n->list);
    printf(" }");
}

EMIT(host_argument) {
    if (n->host) {
	printf("%s", n->host);
    }
    if (n->mask) {
	printf("/%s", n->mask);
    }
}

EMIT(host_argument_list) {
    if (n->list) {
	emit_host_argument_list(n->list);
	printf(" ");
    }
    emit_host_argument(n->arg);
}

EMIT(host_specifier) {
    if (n->type == TOK_SOURCE) {
	printf("source ");
    } else if (n->type == TOK_DEST) {
	printf("dest ");
    }
    printf("{ ");
    if (n->list)
      emit_host_argument_list(n->list);
    printf(" }");
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
	break;
      default:
	printf("error"); break;
    }
}

EMIT(direction_argument) {
    eprint("emitting direction\n");
    if (n->direction)
      printf("%s", n->direction);
    else
      printf("error: n->direction is NULL\n");
}

EMIT(direction_argument_list) {
    if (n->list) {
	eprint("emitting direction_argument_list\n");
	emit_direction_argument_list(n->list);
	printf(" ");
    }
    if (n->arg) {
      eprint("emitting direction_argument\n");
      emit_direction_argument(n->arg);
    }
}

EMIT(direction_specifier) {
    if (n->type == TOK_INPUT) {
	printf("input ");
    } else {
	printf("output ");
    }
      printf("{ ");
    if (n->list) {
      eprint("emitting direction_argument\n");
      emit_direction_argument_list(n->list);
    }
      printf(" }");
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
    } else if (n->option) {
	eprint("emitting routing_specifier\n");
	emit_option_specifier(n->option);
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

int main(int argc __attribute__((unused)), char ** argv __attribute__((unused))) {
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
