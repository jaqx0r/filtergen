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
#define EMITOPTION(x) if (n->x##_option) { emit_##x##_option(n->x##_option); }

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
    if (n->string) {
	char * spaces = strstr(n->string, " ");
	if (spaces)
	    printf("\"");
	printf("%s", n->string);
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

EMIT(range) {
    if (n->start) {
	emit_identifier(n->start);
    }
    printf(":");
    if (n->end) {
	emit_identifier(n->end);
    }
}

EMIT(not_range) {
    if (n->neg) {
	printf("! ");
    }
    if (n->range) {
	eprint("emitting range\n");
	emit_range(n->range);
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

EMIT(destination_option) {
    if (n->not_identifier) {
	printf("-d ");
	emit_not_identifier(n->not_identifier);
    }
}

EMIT(protocol_option) {
    if (n->identifier) {
	printf("-p ");
	emit_identifier(n->identifier);
    }
}

EMIT(match_option) {
    if (n->identifier) {
	printf("-m ");
	emit_identifier(n->identifier);
    }
}

EMIT(dport_option) {
    if (n->not_range) {
	printf("--dport ");
	emit_not_range(n->not_range);
    } else if (n->not_identifier) {
	printf("--dport ");
	emit_not_identifier(n->not_identifier);
    }
}

EMIT(to_ports_option) {
    if (n->identifier) {
	printf("--to-ports ");
	emit_identifier(n->identifier);
    }
}

EMIT(source_option) {
    if (n->not_identifier) {
	printf("-s ");
	emit_not_identifier(n->not_identifier);
    }
}

EMIT(out_interface_option) {
    if (n->not_identifier) {
	printf("-o ");
	emit_not_identifier(n->not_identifier);
    }
}

EMIT(to_source_option) {
    if (n->identifier) {
	printf("--to-source ");
	emit_identifier(n->identifier);
    }
}

EMIT(state_option) {
    if (n->identifier) {
	printf("--state ");
	emit_identifier(n->identifier);
    }
}

EMIT(limit_option) {
    if (n->identifier) {
	printf("--limit ");
	emit_identifier(n->identifier);
    }
}

EMIT(log_prefix_option) {
    if (n->identifier) {
	printf("--log-prefix ");
	emit_identifier(n->identifier);
    }
}

EMIT(sport_option) {
    if (n->not_range) {
	printf("--sport ");
	emit_not_range(n->not_range);
    } else if (n->not_identifier) {
	printf("--sport ");
	emit_not_identifier(n->not_identifier);
    }
}

EMIT(uid_owner_option) {
    if (n->identifier) {
	printf("--uid-owner ");
	emit_identifier(n->identifier);
    }
}

EMIT(tcp_flags_option) {
    if (n->flags) {
	printf("--tcp-flags ");
	emit_identifier(n->flags);
	if (n->mask) {
	    printf(" ");
	    emit_identifier(n->mask);
	}
    }
}

EMIT(reject_with_option) {
    if (n->identifier) {
	printf("--reject-with ");
	emit_identifier(n->identifier);
    }
}

EMIT(icmp_type_option) {
    if (n->identifier) {
	printf("--icmp-type ");
	emit_identifier(n->identifier);
    }
}

EMIT(fragment_option) {
    if (n) {
	printf("-f");
    }
}

EMIT(option) {
    if (n->in_interface_option) {
      emit_in_interface_option(n->in_interface_option);
    } else if (n->jump_option) {
      emit_jump_option(n->jump_option);
    } else if (n->destination_option) {
	emit_destination_option(n->destination_option);
    } else if (n->protocol_option) {
	emit_protocol_option(n->protocol_option);
    } else if (n->match_option) {
	emit_match_option(n->match_option);
    } else if (n->dport_option) {
	emit_dport_option(n->dport_option);
    } else if (n->to_ports_option) {
	emit_to_ports_option(n->to_ports_option);
    } else if (n->source_option) {
	emit_source_option(n->source_option);
    } else if (n->out_interface_option) {
	emit_out_interface_option(n->out_interface_option);
    }
    else EMITOPTION(to_source)
    else EMITOPTION(state)
    else EMITOPTION(limit)
    else EMITOPTION(log_prefix)
    else EMITOPTION(sport)
    else EMITOPTION(uid_owner)
    else EMITOPTION(tcp_flags)
    else EMITOPTION(reject_with)
    else EMITOPTION(icmp_type)
    else EMITOPTION(fragment)
    else {
	eprint("WARNING: no option\n");
    }
}

EMIT(not_option) {
    if (n->neg) {
	printf("! ");
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
    if (n->option_list) {
	if (n->pkt_count) {
	    eprint("emitting pkt_count\n");
	    emit_pkt_count(n->pkt_count);
	    printf(" ");
	}
	printf("-A %s ", n->chain);
	eprint("emitting option_list\n");
	emit_option_list(n->option_list);
    } else if (n->chain) {
	printf(":%s", n->chain);
	if (n->policy) {
	    printf(" %s", n->policy);
	}
	if (n->pkt_count) {
	    printf(" ");
	    eprint("emitting pkt_count\n");
	    emit_pkt_count(n->pkt_count);
	}
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
