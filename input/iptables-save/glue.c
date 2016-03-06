/* conversion glue between iptables-save ast and filtergen internal representation
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

#include "filter.h"
#include "ast.h"
#include "parser.h"

int yyparse(void *);
int yyrestart(FILE *);

int ipts_convtrace = 0;

#define eprint(x) if (ipts_convtrace) fprintf(stderr, x)

struct filter * ipts_convert_rule_list(struct rule_list_s * n) {
    struct filter * res = NULL;

    if (n->list) {
	res = ipts_convert_rule_list(n->list);
    }
    return res;
}

struct filter * ipts_convert(struct ast_s * ast) {
    struct filter * res = NULL;

    eprint("converting ast\n");

    if (ast->list)
	res = ipts_convert_rule_list(ast->list);
    return res;
}

struct filter * ipts_filter_parse_list(void) {
    struct filter * f = NULL;
    struct ast_s ast;
    int r;

    /* parse the input */
    r = yyparse((void *) &ast);
    if (r != 0) {
	printf("iptables-save parser failed: %d\n", r);
    }

    /* convert iptables-save AST into filtergen IR */
    if (!(f = ipts_convert(&ast))) {
	printf("iptables-save conversion failed!\n");
    }

    return f;
}
