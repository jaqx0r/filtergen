/* ast emitter
 * output should be identical (syntactically) to the input
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast.h"
#include "../parser.h"

int yyparse(void *);
int emittrace = 0;

#define EMIT(x) void emit_##x(struct x##_s * n)

#define eprint(x) if (emittrace) fprintf(stderr, x)

EMIT(pkt_count) {
    printf("[");
    if (n->in) {
	printf("%s", n->in);
    }
    printf(":");
    if (n->out) {
	printf("%s", n->out);
    }
    printf("]");
}

EMIT(identifier) {
    if (n->id1) {
	char * spaces = strstr(n->id1, " ");
	if (spaces)
	    printf("\"");
	printf("%s", n->id1);
	if (spaces)
	    printf("\"");
    }
    if (n->id2) {
	printf(" ");
	char * spaces = strstr(n->id2, " ");
	if (spaces)
	    printf("\"");
	printf("%s", n->id2);
	if (spaces)
	    printf("\"");
    }
}

EMIT(not_identifier) {
    if (n->neg) {
	printf("! ");
    }
    if (n->identifier) {
	eprint("emitting identifier\n");
	emit_identifier(n->identifier);
    }
}

EMIT(in_interface_option) {
    if (n->not_identifier) {
	printf("-i ");
	emit_not_identifier(n->not_identifier);
    }
}

EMIT(jump_option) {
  if (n->identifier) {
    printf("-j ");
    emit_identifier(n->identifier);
  }
}

EMIT(option) {
    if (n->in_interface_option) {
      emit_in_interface_option(n->in_interface_option);
    } else if (n->jump_option) {
      emit_jump_option(n->jump_option);
    }
}

EMIT(not_option) {
    if (n->neg) {
	printf("!");
    }
    if (n->option) {
	eprint("emitting option\n");
	emit_option(n->option);
    }
}

EMIT(option_list) {
    if (n->option_list) {
	eprint("emitting option_list\n");
	emit_option_list(n->option_list);
	printf(" ");
    }
    if (n->not_option) {
	eprint("emitting not_option\n");
	emit_not_option(n->not_option);
    }
}

EMIT(rule) {
    if (n->chain) {
	printf(":%s", n->chain);
	if (n->policy) {
	    printf(" %s", n->policy);
	}
	if (n->pkt_count) {
	    printf(" ");
	    eprint("emitting pkt_count\n");
	    emit_pkt_count(n->pkt_count);
	}
    } else if (n->option_list) {
	if (n->pkt_count) {
	    eprint("emitting pkt_count\n");
	    emit_pkt_count(n->pkt_count);
	}
	eprint("emitting option_list\n");
	emit_option_list(n->option_list);
    }
    /* rules end in a newline */
    printf("\n");
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

EMIT(table) {
  if (n->name) {
    printf("*%s\n", n->name);
  }
  if (n->rule_list) {
    eprint("emitting rule_list\n");
    emit_rule_list(n->rule_list);
  }
  printf("COMMIT\n");
}

EMIT(table_list) {
  if (n->list) {
    eprint("emitting table_list\n");
    emit_table_list(n->list);
  }
  if (n->table) {
    eprint ("emitting table\n");
    emit_table(n->table);
  }
}

EMIT(ast) {
    if (n->list) {
	eprint("emitting rule_list\n");
	emit_table_list(n->list);
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
