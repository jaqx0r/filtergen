#include <stdio.h>
#include "filter.h"
#include "ast.h"
#include "parser.h"

int yyparse(void *);
int yyrestart(FILE *);

int convtrace = 1;

int filter_fopen(const char * filename) {
    FILE * file;

    if (filename) {
	/* XXX - make more effort to find file */
	if (!(file = fopen(filename, "r"))) {
	    printf("can't open file \"%s\"", filename);
	    return -1;
	}
    } else {
	file = stdin;
    }
    yyrestart(file);
    return 0;
}

#define CONVERT(x) struct filter * convert_##x(struct x##_s * n)

#define eprint(x) if (convtrace) fprintf(stderr, x)

CONVERT(subrule_list) {
    struct filter * res = NULL;
    /*    struct filter * list = NULL; */

    eprint("converting subrule_list\n");
    /*
    if (n->subrule_list) {
	list = new_filter_sibs(NULL);
    }
    */
    return res;
}

struct filter * convert_compound_specifier(struct compound_specifier_s * r) {
    struct filter * res = NULL;

    eprint("converting compound_specifier\n");

    if (r->list) {
	res = convert_subrule_list(r->list);
    }
    return res;
}

struct filter * convert_specifier(struct specifier_s * r) {
    struct filter * res = NULL;
    eprint("converting specifier\n");

    if (r->compound) {
      eprint("converting compound specifier\n");
	res = convert_compound_specifier(r->compound);
    } else if (r->direction) {
	enum filtertype type;

	eprint("converting direction specifier\n");

	switch (r->direction->type) {
	  case TOK_INPUT:
	    type = INPUT;
	    break;
	  case TOK_OUTPUT:
	    type = OUTPUT;
	    break;
	  default:
	    printf("error: incorrect direction type encountered\n");
	    type = YYEOF;
	    break;
	}
	if (r->direction->arg) {
	    if (r->direction->arg->simple) {
		if (r->direction->arg->simple->identifier) {
		    res = new_filter_device(type, r->direction->arg->simple->identifier);
		} else {
		    printf("error: no direction argument (simple) identifier\n");
		}
	    } else {
		printf("error: no direction argument (simple)\n");
	    }
	} else {
	    printf("error: no direction argument\n");
	}
    } else if (r->target) {
      enum filtertype type;

      eprint("converting target specifier\n");

      switch (r->target->type) {
       case TOK_ACCEPT:
	type = T_ACCEPT;
	break;
       case TOK_REJECT:
	type = T_REJECT;
	break;
       case TOK_DROP:
	type = DROP;
	break;
       case TOK_MASQ:
	type = MASQ;
	break;
       case TOK_PROXY:
	type = REDIRECT;
	break;
       case TOK_REDIRECT:
	type = REDIRECT;
	break;
       default:
	printf("error: incorrect target type encountered\n");
	type = YYEOF;
	break;
      }
      res = new_filter_target(type);
    } else if (r->host) {
	enum filtertype type;
	
	eprint("converting host specifier\n");

	switch (r->host->type) {
	  case TOK_SOURCE:
	    type = F_SOURCE;
	    break;
	  case TOK_DEST:
	    type = F_DEST;
	    break;
	  default:
	    printf("error: incorrect host type encountered\n");
	    type = YYEOF;
	    break;
	}
	if (r->host->arg) {
	    if (r->host->arg->simple) {
		if (r->host->arg->simple->host) {
		  eprint("creating new filter host: ");
		  eprint(r->host->arg->simple->host->host);
		  eprint("\n");
		  if (r->host->arg->simple->host->host) {
		    
		    res = new_filter_host(type, r->host->arg->simple->host->host);
		  } else {
		    printf("error: no actual contents to the host argument (simple) identifier\n");
		  }
		} else {
		    printf("error: no host argument (simple) identifier\n");
		}
	    } else {
		printf("error: no host argument (simple)\n");
	    }
	} else {
	    printf("error: no host argument\n");
	}
    } else if (r->port) {
      eprint("converting port specifier\n");
	res = new_filter_ports(F_SPORT, "bar");
    } else if (r->protocol) {
      eprint("converting proto specifier\n");
	res = new_filter_proto(F_PROTO, "53");
    } else if (r->icmptype) {
      eprint("converting icmp specifier\n");
	res = __new_filter(F_SIBLIST);
    } else if (r->routing) {
      eprint("converting routing specifier\n");
	res = __new_filter(F_SUBGROUP);
    } else if (r->chaingroup) {
      eprint("converting chaingroup specifier\n");
	res = __new_filter(F_RTYPE);
    } else
	printf("error: no specifiers\n");
    
    return res;
}

struct filter * convert_negated_specifier(struct negated_specifier_s * r) {
    struct filter * spec = NULL;
    struct filter * res = NULL;

    eprint("converting negated specifier\n");

    if (r->spec) {
	spec = convert_specifier(r->spec);
	if (spec && r->negated) {
	  eprint("negating\n");
	    res = new_filter_neg(spec);
	} else {
	    res = spec;
	}
    }
    return res;
}

struct filter * convert_specifier_list(struct specifier_list_s * n) {
  struct filter * res = NULL, * end = NULL;

    eprint("converting specifier_list\n");

    if (n->list) {
      res = convert_specifier_list(n->list);
      end = res;
      while (end->child) {
	end = end->child;
      }
      if (n->spec) {
	end->child = convert_negated_specifier(n->spec);
      }
    } else {
      res = convert_negated_specifier(n->spec);
    }

    return res;
}

struct filter * convert_rule(struct rule_s * r) {
    struct filter * res = NULL;

    eprint("converting rule\n");

    if (r->list)
	res = convert_specifier_list(r->list);
    return res;
}

struct filter * convert_rule_list(struct rule_list_s * r) {
    struct filter * res = NULL;
    struct filter * list = NULL;
    struct filter * rule = NULL;

    eprint("converting rule_list\n");

    if (r->list) {
	list = convert_rule_list(r->list);
    }
    if (r->rule) {
	rule = convert_rule(r->rule);
    }
    if (list) {
	list->next = rule;
	res = list;
    } else {
	res = rule;
    }
    return res;
}

struct filter * convert(struct ast_s * ast) {
    struct filter * res = NULL;
    
    eprint("converting ast\n");

    if (ast->list)
	res = convert_rule_list(ast->list);
    return res;
}

struct filter * filter_parse_list(void) {
    struct filter * f = NULL;
    struct ast_s ast;
    int r;

    /* parse the input */
    r = yyparse((void *) &ast);
    if (r != 0) {
	printf("parser failed: %d\n", r);
    }

    /* convert our new style AST into the old style struct */
    if (!(f = convert(&ast))) {
	printf("conversion failed!\n");
    }

    return f;
}
