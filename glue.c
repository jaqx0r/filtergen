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

#define eprint(x) if (convtrace) fprintf(stderr, x)

struct filter * convert_specifier_list(struct specifier_list_s * n);

struct filter * convert_subrule_list(struct subrule_list_s * n) {
    struct filter * res = NULL; /*, * end = NULL; */

    eprint("converting subrule_list\n");

    if (n->subrule_list) {
	res = convert_subrule_list(n->subrule_list);
        if (res && n->specifier_list) {
            res->next = convert_specifier_list(n->specifier_list);
        }
    } else if (n->specifier_list) {
        res = convert_specifier_list(n->specifier_list);
    } else {
        printf("error: no content in subrule_list\n");
    }

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

struct filter * convert_direction_arg(struct simple_direction_argument_s * n, int type) {
    struct filter * res = NULL;

    if (n->identifier) {
        res = new_filter_device(type, n->identifier);
    } else {
        printf("error: no direction argument (simple) identifier\n");
    }

    return res;
}

struct filter * convert_direction_argument_list(struct direction_argument_list_s * n, int type) {
    struct filter * res = NULL, * end = NULL;

    eprint("converting direction argument list\n");

    if (n->list) {
        res = convert_direction_argument_list(n->list, type);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = convert_direction_arg(n->arg, type);
            }
        } else {
            printf("warning: convert_direction_argument_list returned NULL\n");
        }
    } else {
        res = convert_direction_arg(n->arg, type);
    }

    return res;
}
    
struct filter * convert_direction(struct direction_specifier_s * n) {
    struct filter * res = NULL;
    int type;

    eprint("converting direction specifier\n");
        
    switch (n->type) {
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
    if (n->arg) {
        if (n->arg->simple) {
            res = convert_direction_arg(n->arg->simple, type);
        } else if (n->arg->compound) {
            eprint("converting compound direction argument\n");

            if (n->arg->compound->list) {
                res = new_filter_sibs(convert_direction_argument_list(n->arg->compound->list, type));
            } else {
                printf("error: no direction argument (compound) list\n");
            }
        } else {
            printf("error: neither simple nor compound argument\n");
        }
    } else {
        printf("error: no direction argument\n");
    }

    return res;
}

struct filter * convert_host_argument(struct simple_host_argument_s * n, int type) {
    struct filter * res = NULL;

    eprint("converting simple_host_argument\n");

    if (n->host) {
        res = new_filter_host(type, n->host);
        if (n->mask) {
            printf("error: mask not handled\n");
        }
    } else {
        printf("error: no host part\n");
    }

    return res;
}

struct filter * convert_host_argument_list(struct host_argument_list_s * n, int type) {
    struct filter * res = NULL, * end = NULL;

    eprint("converting host argument list\n");

    if (n->list) {
        res = convert_host_argument_list(n->list, type);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = convert_host_argument(n->arg, type);
            }
        } else {
            printf("warning: convert_host_argument_list returned NULL\n");
        }
    } else {
        res = convert_host_argument(n->arg, type);
    }

    return res;
}

struct filter * convert_host_specifier(struct host_specifier_s * n) {
    struct filter * res = NULL;
    enum filtertype type;
	
    eprint("converting host specifier\n");

    switch (n->type) {
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
    if (n->arg) {
        if (n->arg->simple) {
            res = convert_host_argument(n->arg->simple, type);
        } else if (n->arg->compound) {
            if (n->arg->compound->list) {
                res = new_filter_sibs(convert_host_argument_list(n->arg->compound->list, type));
            } else {
                printf("error: no host argument (compound) list\n");
            }
        } else {
            printf("error: neither simple nor compound argument\n");
        }
    } else {
        printf("error: no host argument\n");
    }

    return res;        
}

struct filter * convert_protocol_argument(struct simple_protocol_argument_s * n) {
    struct filter * res = NULL;

    eprint("converting simple_protocol_argument\n");

    if (n->proto) {
        res = new_filter_proto(F_PROTO, n->proto);
    } else {
        printf("error: no proto part\n");
    }

    return res;
}

struct filter * convert_protocol_argument_list(struct protocol_argument_list_s * n) {
    struct filter * res = NULL, * end = NULL;

    eprint("converting protocol argument list\n");

    if (n->list) {
        res = convert_protocol_argument_list(n->list);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = convert_protocol_argument(n->arg);
            }
        } else {
            printf("warning: convert_protocol_argument_list returned NULL\n");
        }
    } else {
        res = convert_protocol_argument(n->arg);
    }

    return res;
}

struct filter * convert_protocol_specifier(struct protocol_specifier_s * n) {
    struct filter * res = NULL;
	
    eprint("converting protocol specifier\n");

    if (n->arg) {
        if (n->arg->simple) {
            res = convert_protocol_argument(n->arg->simple);
        } else if (n->arg->compound) {
            if (n->arg->compound->list) {
                res = new_filter_sibs(convert_protocol_argument_list(n->arg->compound->list));
            } else {
                printf("error: no protocol argument (compound) list\n");
            }
        } else {
            printf("error: neither simple nor compound argument\n");
        }
    } else {
        printf("error: no protocol argument\n");
    }

    return res;        
}

struct filter * convert_port_argument(struct simple_port_argument_s * n, int type) {
    struct filter * res = NULL;
    char * p;

    eprint("converting simple_port_argument\n");

    if (n->port_min) {
        if (n->port_max) {
            asprintf(&p, "%s:%s", n->port_min, n->port_max);
            res = new_filter_ports(type, p);
        } else {
            res = new_filter_ports(type, n->port_min);
        }
    } else {
        printf("error: no port argument contents\n");
    }

    return res;
}

struct filter * convert_port_argument_list(struct port_argument_list_s * n, int type) {
    struct filter * res = NULL, * end = NULL;

    eprint("converting port argument list\n");

    if (n->list) {
        res = convert_port_argument_list(n->list, type);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = convert_port_argument(n->arg, type);
            }
        } else {
            printf("warning: convert_port_argument_list returned NULL\n");
        }
    } else {
        res = convert_port_argument(n->arg, type);
    }

    return res;
}

struct filter * convert_port_specifier(struct port_specifier_s * n) {
    struct filter * res = NULL;
    enum filtertype type;
	
    eprint("converting port specifier\n");

    switch (n->type) {
    case TOK_SPORT:
        type = F_SPORT;
        break;
    case TOK_DPORT:
        type = F_DPORT;
        break;
    default:
        printf("error: incorrect port type encountered\n");
        type = YYEOF;
        break;
    }
    if (n->arg) {
        if (n->arg->simple) {
            res = convert_port_argument(n->arg->simple, type);
        } else if (n->arg->compound) {
            if (n->arg->compound->list) {
                res = new_filter_sibs(convert_port_argument_list(n->arg->compound->list, type));
            } else {
                printf("error: no port argument (compound) list\n");
            }
        } else {
            printf("error: neither simple nor compound argument\n");
        }
    } else {
        printf("error: no port argument\n");
    }

    return res;        
}

struct filter * convert_icmptype_argument(struct simple_icmptype_argument_s * n) {
    struct filter * res = NULL;

    eprint("converting simple_icmptype_argument\n");

    if (n->icmptype) {
        res = new_filter_icmp(F_ICMPTYPE, n->icmptype);
    } else {
        printf("error: no icmptype argument contents\n");
    }

    return res;
}

struct filter * convert_icmptype_argument_list(struct icmptype_argument_list_s * n) {
    struct filter * res = NULL, * end = NULL;

    eprint("converting icmptype argument list\n");

    if (n->list) {
        res = convert_icmptype_argument_list(n->list);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = convert_icmptype_argument(n->arg);
            }
        } else {
            printf("warning: convert_icmptype_argument_list returned NULL\n");
        }
    } else {
        res = convert_icmptype_argument(n->arg);
    }

    return res;
}

struct filter * convert_icmptype_specifier(struct icmptype_specifier_s * n) {
    struct filter * res = NULL;
	
    eprint("converting icmptype specifier\n");

    if (n->arg) {
        if (n->arg->simple) {
            res = convert_icmptype_argument(n->arg->simple);
        } else if (n->arg->compound) {
            if (n->arg->compound->list) {
                res = new_filter_sibs(convert_icmptype_argument_list(n->arg->compound->list));
            } else {
                printf("error: no icmptype argument (compound) list\n");
            }
        } else {
            printf("error: neither simple nor compound argument\n");
        }
    } else {
        printf("error: no icmptype argument\n");
    }

    return res;        
}

struct filter * convert_routing_specifier(struct routing_specifier_s * n) {
    struct filter * res = NULL;
    enum filtertype type;
	
    eprint("converting routing specifier\n");

    switch (n->type) {
      case TOK_LOCAL:
	type = LOCALONLY;
	break;
      case TOK_FORWARD:
	type = ROUTEDONLY;
	break;
      default:
	printf("error: incorrect routing type encountered\n");
	type = YYEOF;
	break;
    }
    res = new_filter_rtype(type);

    return res;        
}

struct filter * convert_specifier(struct specifier_s * r) {
    struct filter * res = NULL;
    eprint("converting specifier\n");
    
    if (r->compound) {
        eprint("converting compound specifier\n");
	res = convert_compound_specifier(r->compound);
    } else if (r->direction) {
        res = convert_direction(r->direction);
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
        res = convert_host_specifier(r->host);
    } else if (r->protocol) {
        res = convert_protocol_specifier(r->protocol);
    } else if (r->port) {
        res = convert_port_specifier(r->port);
    } else if (r->icmptype) {
        res = convert_icmptype_specifier(r->icmptype);
    } else if (r->routing) {
      res = convert_routing_specifier(r->routing);
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
      if (res) {
          end = res;
          while (end->child) {
              end = end->child;
          }
          if (n->spec) {
              end->child = convert_negated_specifier(n->spec);
          }
      } else {
          printf("warning: convert_specifier_list returned NULL\n");
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

struct filter * convert_rule_list(struct rule_list_s * n) {
  struct filter * res = NULL, * end = NULL;

    eprint("converting rule_list\n");

    if (n->list) {
	res = convert_rule_list(n->list);
	end = res;
	while (end->next) {
	  end = end->next;
	}
	if (n->rule) {
	  end->next = convert_rule(n->rule);
	}
    } else {
      res = convert_rule(n->rule);
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
