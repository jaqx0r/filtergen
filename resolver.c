/* argument name resolver
 *
 * Copyright (c) 2004 Jamie Wilkinson <jaq@spacepants.org>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "resolver.h"
#include "ast.h"
#include "icmpent.h"

void resolve_option_specifier(struct option_specifier_s * n __attribute__((unused))) {
}

void resolve_icmptype_argument(struct icmptype_argument_s * n) {
    struct icmpent_s * i;

    if (n->icmptype) {
        if ((i = geticmpbyname(n->icmptype))) {
            free(n->icmptype);
            asprintf(&n->icmptype, "%s", i->i_type);
        } else {
	    /* check that the icmptype is a number if we can't resolve it */
	    long m;
	    char * e;

	    m = strtol(n->icmptype, &e, 10);
	    if (*e) {
		fprintf(stderr, "warning: suspicious icmp type encountered: %s\n", n->icmptype);
	    }
        }        
    }
}

void resolve_icmptype_argument_list(struct icmptype_argument_list_s * n) {
    if (n->list) {
        resolve_icmptype_argument_list(n->list);
    }
    if (n->arg) {
        resolve_icmptype_argument(n->arg);
    }
}

void resolve_icmptype_specifier(struct icmptype_specifier_s * n) {
    if (n->list) {
	resolve_icmptype_argument_list(n->list);
    }
}

void resolve_port_argument(struct port_argument_s * n) {
    struct servent * s;

    if (n->port_min) {
	/* try to resolve the port name */
	if ((s = getservbyname(n->port_min, NULL))) {
	    free(n->port_min);
	    asprintf(&n->port_min, "%d", ntohs(s->s_port));
	} else {
	    /* check that the port is a number if we can't resolve it */
	    long m;
	    char * e;

	    m = strtol(n->port_min, &e, 10);
	    if (*e) {
		fprintf(stderr, "warning: suspicious port name encountered: %s\n", n->port_min);
	    }
	}
    }
    if (n->port_max) {
	/* try to resolve the port name */
	if ((s = getservbyname(n->port_max, NULL))) {
	    free(n->port_max);
	    asprintf(&n->port_max, "%d", ntohs(s->s_port));
	} else {
	    /* check that the port is a number if we can't resolve it */
	    long m;
	    char * e;

	    m = strtol(n->port_max, &e, 10);
	    if (*e) {
		fprintf(stderr, "warning: suspicious port name encountered: %s\n", n->port_max);
	    }
	}
    }
}

void resolve_port_argument_list(struct port_argument_list_s * n) {
    if (n->list) {
	resolve_port_argument_list(n->list);
    }
    if (n->arg) {
	resolve_port_argument(n->arg);
    }
}

void resolve_port_specifier(struct port_specifier_s * n) {
    if (n->list) {
	resolve_port_argument_list(n->list);
    }
}

void resolve_protocol_argument(struct protocol_argument_s * n) {
    struct protoent * p;

    if (n->proto) {
	if ((p = getprotobyname(n->proto))) {
	    free(n->proto);
	    asprintf(&n->proto, "%d", p->p_proto);
	} else {
	    /* check that the proto is a number if we can't resolve it */
	    long m;
	    char * e;

	    m = strtol(n->proto, &e, 10);
	    if (*e) {
		fprintf(stderr, "warning: suspicious protocol name encountered: %s\n", n->proto);
	    }
	}
    }
}

void resolve_protocol_argument_list(struct protocol_argument_list_s * n) {
    if (n->list) {
	resolve_protocol_argument_list(n->list);
    }
    if (n->arg) {
	resolve_protocol_argument(n->arg);
    }
}

void resolve_protocol_specifier(struct protocol_specifier_s * n) {
    if (n->list) {
	resolve_protocol_argument_list(n->list);
    }
}

void resolve_host_argument(struct host_argument_s * n __attribute__((unused))) {
}

void resolve_host_argument_list(struct host_argument_list_s * n) {
    struct addrinfo * a = NULL, * i;
    struct addrinfo hints;
    int r;
    struct host_argument_list_s * list = NULL;
    struct host_argument_s * host = NULL;

    if (n->list) {
        resolve_host_argument_list(n->list);
    }

    if (n->arg) {
        memset(&hints, 0, sizeof(struct addrinfo));
        /* any address family is good */
        hints.ai_family = PF_UNSPEC;
        /* return hostname, though we don't use it, for debugging */
        hints.ai_flags = AI_CANONNAME;
        /* limit so duplicate hosts aren't returned for each socktype */
        hints.ai_socktype = SOCK_STREAM;
    
        if (n->arg->host) {
            r = getaddrinfo(n->arg->host, NULL, &hints, &a);
            switch (r) {
              case 0:
                /* replace the hostname with the IP */
                free(n->arg->host);
		/* getnameinfo does no allocation. */
		n->arg->host = malloc(NI_MAXHOST + 1);
		if (getnameinfo(a->ai_addr, a->ai_addrlen, n->arg->host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
		    /* if there's more, create some more hosts */
		    for (i = a->ai_next; i; i = i->ai_next) {
			list = malloc(sizeof(struct host_argument_list_s));
			host = malloc(sizeof(struct host_argument_s));
			host->host = malloc(NI_MAXHOST + 1);
			if (getnameinfo(i->ai_addr, i->ai_addrlen, host->host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
			    if (n->arg->mask) {
				host->mask = strdup(n->arg->mask);
			    }

			    /* insert the new node */
			    list->arg = host;
			    list->list = n->list;
			    n->list = list;
			} else {
			    fprintf(stderr, "warning: %s\n", strerror(errno));
			}
		    }
		} else {
		    fprintf(stderr, "warning: %s\n", strerror(errno));
		}
                freeaddrinfo(a);
                break;
              default:
                fprintf(stderr, "warning: %s: %s\n", gai_strerror(r), n->arg->host);
                break;
            }
        }
    }
}

void resolve_host_specifier(struct host_specifier_s * n) {
    if (n->list) {
        resolve_host_argument_list(n->list);
    }
}

void resolve_target_specifier(struct target_specifier_s * n __attribute__((unused))) {
}

void resolve_direction_argument(struct direction_argument_s * n __attribute__((unused))) {
}

void resolve_direction_argument_list(struct direction_argument_list_s * n) {
    if (n->list) {
	resolve_direction_argument_list(n->list);
    }
    if (n->arg) {
	resolve_direction_argument(n->arg);
    }
}

void resolve_direction_specifier(struct direction_specifier_s * n) {
    if (n->list) {
	resolve_direction_argument_list(n->list);
    }
}

void resolve_specifier_list(struct specifier_list_s * n);

void resolve_subrule_list(struct subrule_list_s * n) {
    if (n->subrule_list) {
        resolve_subrule_list(n->subrule_list);
    }
    if (n->specifier_list) {
        resolve_specifier_list(n->specifier_list);
    }
}

void resolve_chaingroup_specifier(struct chaingroup_specifier_s * n) {
    if (n->list) {
        resolve_subrule_list(n->list);
    }
}

void resolve_compound_specifier(struct compound_specifier_s * n) {
    if (n->list) {
        resolve_subrule_list(n->list);
    }
}

void resolve_specifier(struct specifier_s * n) {
    if (n->compound) {
	resolve_compound_specifier(n->compound);
    } else if (n->direction) {
	resolve_direction_specifier(n->direction);
    } else if (n->target) {
	resolve_target_specifier(n->target);
    } else if (n->host) {
	resolve_host_specifier(n->host);
    } else if (n->port) {
	resolve_port_specifier(n->port);
    } else if (n->protocol) {
	resolve_protocol_specifier(n->protocol);
    } else if (n->icmptype) {
	resolve_icmptype_specifier(n->icmptype);
    } else if (n->option) {
	resolve_option_specifier(n->option);
    } else if (n->chaingroup) {
	resolve_chaingroup_specifier(n->chaingroup);
    }
}

void resolve_negated_specifier(struct negated_specifier_s * n) {
    if (n->spec) {
	resolve_specifier(n->spec);
    }
}

void resolve_specifier_list(struct specifier_list_s * n) {
    if (n->list) {
	resolve_specifier_list(n->list);
    }
    if (n->spec) {
	resolve_negated_specifier(n->spec);
    }
}

void resolve_rule(struct rule_s * n) {
    if (n->list) {
	resolve_specifier_list(n->list);
    }
}

void resolve_rule_list(struct rule_list_s * n) {
    if (n->list) {
	resolve_rule_list(n->list);
    }
    if (n->rule) {
	resolve_rule(n->rule);
    }
}

void resolve_ast(struct ast_s * n) {
    if (n->list) {
	resolve_rule_list(n->list);
    }
}

void resolve(struct ast_s * n) {
    if (n) {
	resolve_ast(n);
    }
}
