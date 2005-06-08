/* conversion glue between ast and filter structures
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

#include <stdio.h>
#include <assert.h>
#include "input/input.h"
#include "ir/ir.h"
#include "ast.h"
#include "parser.h"
#include "resolver.h"

int filtergen_parse(void *);
int filtergen_restart(FILE *);

int convtrace = 1;

#define eprint(x) if (convtrace) fprintf(stderr, x)

#if 0

struct ir_s * filtergen_convert_specifier_list(struct specifier_list_s * n);

struct ir_s * filtergen_convert_subrule_list(struct subrule_list_s * n) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting subrule_list\n");

    if (n->subrule_list) {
	res = filtergen_convert_subrule_list(n->subrule_list);
	if (res) {
	    end = res;
	    while (end->next) {
		end = end->next;
	    }
	    if (n->specifier_list) {
		end->next = filtergen_convert_specifier_list(n->specifier_list);
	    }
	} else {
	    printf("warning: filtergen_convert_subrule_list returned NULL\n");
	}
    } else if (n->specifier_list) {
        res = filtergen_convert_specifier_list(n->specifier_list);
    } else {
        printf("error: no content in subrule_list\n");
    }

    return res;
}

struct ir_s * filtergen_convert_compound_specifier(struct compound_specifier_s * r) {
    struct ir_s * res = NULL;

    eprint("filtergen_converting compound_specifier\n");

    if (r->list) {
	res = new_filter_sibs(filtergen_convert_subrule_list(r->list));
    }
    return res;
}

struct ir_s * filtergen_convert_direction_argument(struct direction_argument_s * n, int type) {
    struct ir_s * res = NULL;

    if (n->direction) {
        res = new_filter_device(type, n->direction);
    } else {
        printf("error: no direction argument contents\n");
    }

    return res;
}

struct ir_s * filtergen_convert_direction_argument_list(struct direction_argument_list_s * n, int type) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting direction argument list\n");

    if (n->list) {
        res = filtergen_convert_direction_argument_list(n->list, type);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = filtergen_convert_direction_argument(n->arg, type);
            }
        } else {
            printf("warning: filtergen_convert_direction_argument_list returned NULL\n");
        }
    } else {
        res = filtergen_convert_direction_argument(n->arg, type);
    }

    return res;
}
    
struct ir_s * filtergen_convert_direction(struct direction_specifier_s * n) {
    struct ir_s * res = NULL;
    int type;

    eprint("filtergen_converting direction specifier\n");
        
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
    if (n->list) {
	res = new_filter_sibs(filtergen_convert_direction_argument_list(n->list, type));
    } else {
	printf("error: no direction argument list\n");
    }

    return res;
}

struct ir_s * filtergen_convert_host_argument(struct host_argument_s * n, int type) {
    struct ir_s * res = NULL;
    char * h;

    eprint("filtergen_converting host_argument\n");

    if (n->host) {
        if (n->mask) {
	    asprintf(&h, "%s/%s", n->host, n->mask);
	    res = new_filter_host(type, h);
	} else {
	    res = new_filter_host(type, n->host);
	}
    } else {
        printf("error: no host part\n");
    }

    return res;
}

struct ir_s * filtergen_convert_host_argument_list(struct host_argument_list_s * n, int type) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting host argument list\n");

    if (n->list) {
        res = filtergen_convert_host_argument_list(n->list, type);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = filtergen_convert_host_argument(n->arg, type);
            }
        } else {
            printf("warning: filtergen_convert_host_argument_list returned NULL\n");
        }
    } else {
        res = filtergen_convert_host_argument(n->arg, type);
    }

    return res;
}

struct ir_s * filtergen_convert_host_specifier(struct host_specifier_s * n) {
    struct ir_s * res = NULL;
    enum filtertype type;
	
    eprint("filtergen_converting host specifier\n");

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
    if (n->list) {
	res = new_filter_sibs(filtergen_convert_host_argument_list(n->list, type));
    } else {
	printf("error: no host argument list\n");
    }

    return res;        
}

struct ir_s * filtergen_convert_protocol_argument(struct protocol_argument_s * n) {
    struct ir_s * res = NULL;

    eprint("filtergen_converting protocol argument\n");

    if (n->proto) {
        res = new_filter_proto(F_PROTO, n->proto);
    } else {
        printf("error: no protocol argument contents\n");
    }

    return res;
}

struct ir_s * filtergen_convert_protocol_argument_list(struct protocol_argument_list_s * n) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting protocol argument list\n");

    if (n->list) {
        res = filtergen_convert_protocol_argument_list(n->list);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = filtergen_convert_protocol_argument(n->arg);
            }
        } else {
            printf("warning: filtergen_convert_protocol_argument_list returned NULL\n");
        }
    } else {
        res = filtergen_convert_protocol_argument(n->arg);
    }

    return res;
}

struct ir_s * filtergen_convert_protocol_specifier(struct protocol_specifier_s * n) {
    struct ir_s * res = NULL;
	
    eprint("filtergen_converting protocol specifier\n");

    if (n->list) {
	res = new_filter_sibs(filtergen_convert_protocol_argument_list(n->list));
    } else {
	printf("error: no protocol argument list\n");
    }

    return res;        
}

struct ir_s * filtergen_convert_port_argument(struct port_argument_s * n, int type) {
    struct ir_s * res = NULL;
    char * p;

    eprint("filtergen_converting port argument\n");

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

struct ir_s * filtergen_convert_port_argument_list(struct port_argument_list_s * n, int type) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting port argument list\n");

    if (n->list) {
        res = filtergen_convert_port_argument_list(n->list, type);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = filtergen_convert_port_argument(n->arg, type);
            }
        } else {
            printf("warning: filtergen_convert_port_argument_list returned NULL\n");
        }
    } else {
        res = filtergen_convert_port_argument(n->arg, type);
    }

    return res;
}

struct ir_s * filtergen_convert_port_specifier(struct port_specifier_s * n) {
    struct ir_s * res = NULL;
    enum filtertype type;
	
    eprint("filtergen_converting port specifier\n");

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
    if (n->list) {
	res = new_filter_sibs(filtergen_convert_port_argument_list(n->list, type));
    } else {
	printf("error: no port argument list\n");
    }

    return res;        
}

struct ir_s * filtergen_convert_icmptype_argument(struct icmptype_argument_s * n) {
    struct ir_s * res = NULL;

    eprint("filtergen_converting icmptype_argument\n");

    if (n->icmptype) {
        res = new_filter_icmp(F_ICMPTYPE, n->icmptype);
    } else {
        printf("error: no icmptype argument contents\n");
    }

    return res;
}

struct ir_s * filtergen_convert_icmptype_argument_list(struct icmptype_argument_list_s * n) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting icmptype argument list\n");

    if (n->list) {
        res = filtergen_convert_icmptype_argument_list(n->list);
        if (res) {
            end = res;
            while (end->next) {
                end = end->next;
            }
            if (n->arg) {
                end->next = filtergen_convert_icmptype_argument(n->arg);
            }
        } else {
            printf("warning: filtergen_convert_icmptype_argument_list returned NULL\n");
        }
    } else {
        res = filtergen_convert_icmptype_argument(n->arg);
    }

    return res;
}

struct ir_s * filtergen_convert_icmptype_specifier(struct icmptype_specifier_s * n) {
    struct ir_s * res = NULL;
	
    eprint("filtergen_converting icmptype specifier\n");

    if (n->list) {
	res = new_filter_sibs(filtergen_convert_icmptype_argument_list(n->list));
    } else {
	printf("error: no icmptype argument list\n");
    }

    return res;        
}

struct ir_s * filtergen_convert_option_specifier(struct option_specifier_s * n) {
    struct ir_s * res = NULL;
	
    eprint("filtergen_converting option specifier\n");

    switch (n->type) {
      case TOK_LOCAL:
	res = new_filter_rtype(LOCALONLY);
	break;
      case TOK_FORWARD:
	res = new_filter_rtype(ROUTEDONLY);
	break;
      case TOK_ONEWAY:
	res = new_filter_rtype(F_ONEWAY);
	break;
      case TOK_LOG:
	res = new_filter_log(F_LOG, n->logmsg);
	break;
      default:
	printf("error: incorrect option type encountered\n");
	break;
    }

    return res;        
}

struct ir_s * filtergen_convert_chaingroup_specifier(struct chaingroup_specifier_s * n) {
    struct ir_s * res = NULL, * sub = NULL;
    char * name = NULL;

    if (n->name) {
	name = n->name;
    } else {
	/* Allocate a filter name */
	static int ccount = 0;

	asprintf(&name, "chain_%d", ccount++);
    }
  
    if (n->list) {
	sub = filtergen_convert_subrule_list(n->list);

	res = new_filter_subgroup(name, sub);
    } else {
	printf("error: no list in chaingroup\n");
    }

    return res;
}

struct ir_s * filtergen_convert_specifier(struct specifier_s * r) {
    struct ir_s * res = NULL;
    eprint("filtergen_converting specifier\n");
    
    if (r->compound) {
        eprint("filtergen_converting compound specifier\n");
	res = filtergen_convert_compound_specifier(r->compound);
    } else if (r->direction) {
        res = filtergen_convert_direction(r->direction);
    } else if (r->target) {
	enum filtertype type;

	eprint("filtergen_converting target specifier\n");

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
        res = filtergen_convert_host_specifier(r->host);
    } else if (r->protocol) {
        res = filtergen_convert_protocol_specifier(r->protocol);
    } else if (r->port) {
        res = filtergen_convert_port_specifier(r->port);
    } else if (r->icmptype) {
        res = filtergen_convert_icmptype_specifier(r->icmptype);
    } else if (r->option) {
	res = filtergen_convert_option_specifier(r->option);
    } else if (r->chaingroup) {
	res = filtergen_convert_chaingroup_specifier(r->chaingroup);
    } else
	printf("error: no specifiers\n");
    
    return res;
}

struct ir_s * filtergen_convert_negated_specifier(struct negated_specifier_s * r) {
    struct ir_s * spec = NULL;
    struct ir_s * res = NULL;

    eprint("filtergen_converting negated specifier\n");

    if (r->spec) {
	spec = filtergen_convert_specifier(r->spec);
	if (spec && r->negated) {
	    eprint("negating\n");
	    res = new_filter_neg(spec);
	} else {
	    res = spec;
	}
    }
    return res;
}

struct ir_s * filtergen_convert_specifier_list(struct specifier_list_s * n) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting specifier_list\n");

    if (n->list) {
	res = filtergen_convert_specifier_list(n->list);
	if (res) {
	    end = res;
	    while (end->child) {
		end = end->child;
	    }
	    if (n->spec) {
		end->child = filtergen_convert_negated_specifier(n->spec);
	    }
	} else {
	    printf("warning: filtergen_convert_specifier_list returned NULL\n");
	}
    } else {
	res = filtergen_convert_negated_specifier(n->spec);
    }

    return res;
}

struct ir_s * filtergen_convert_rule(struct rule_s * r) {
    struct ir_s * res = NULL;

    eprint("filtergen_converting rule\n");

    if (r->list)
	res = filtergen_convert_specifier_list(r->list);
    return res;
}

struct ir_s * filtergen_convert_rule_list(struct rule_list_s * n) {
    struct ir_s * res = NULL, * end = NULL;

    eprint("filtergen_converting rule_list\n");

    if (n->list) {
	res = filtergen_convert_rule_list(n->list);
	end = res;
	while (end->next) {
	    end = end->next;
	}
	if (n->rule) {
	    end->next = filtergen_convert_rule(n->rule);
	}
    } else {
	res = filtergen_convert_rule(n->rule);
    }

    return res;
}
#endif

struct ir_s * filtergen_convert(struct ast_s * ast) {
    struct ir_s * ir = NULL;

    eprint("filtergen_converting ast\n");

    assert(ast);

    ir = ir_new();

    /*
    if (ast->list)
	res = filtergen_convert_rule_list(ast->list);
    */
    
    return ir;
}

struct ir_s * filtergen_source_parser(FILE * file, int resolve_names) {
    struct ast_s ast;
    struct ir_s * ir;

    filtergen_restart(file);
    
    if (filtergen_parse((void *) &ast) != 0) {
	fprintf(stderr, "filtergen parse failed\n");
    } else {
	
	if (resolve_names)
	    resolve(&ast);
	
	if (!(ir = filtergen_convert(&ast))) {
	    fprintf(stderr, "filtergen IR conversion failed\n");
	}
    }

    return ir;
}
