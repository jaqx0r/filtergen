/* parser for iptables-save format
 *
 * Copyright (c) 2003 Jamie Wilkinson <jaq@spacepants.org>
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

/* prepend all functions with ipts_ to keep the namespace separate
 * from other parsers */
%name-prefix="ipts_"

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#define YYPARSE_PARAM parm

void ipts_error(const char *);
extern int ipts_lineno;
extern int ipts_lex(void);

#define YYPRINT(f, t, v) ipts_print(f, t, v)
%}
%debug

%union {
    struct rule_list_s * u_rule_list;
    struct rule_s * u_rule;
    struct option_list_s * u_option_list;
    struct not_option_s * u_not_option;
    struct option_s * u_option;
    struct not_identifier_s * u_not_identifier;
    struct identifier_s * u_identifier;
    struct pkt_count_s * u_pkt_count;
    char * u_str;
}
%type <u_rule_list> rule_list
%type <u_rule> rule
%type <u_option_list> option_list
%type <u_not_option> not_option
%type <u_option> option
%type <u_not_identifier> not_identifier
%type <u_identifier> identifier
%type <u_pkt_count> pkt_count

%defines
%token TOK_IPTS_TABLE
%token TOK_IPTS_CHAIN

%token TOK_IPTS_CHAIN_APPEND

%token TOK_IPTS_PROTOCOL
%token TOK_IPTS_SOURCE
%token TOK_IPTS_DESTINATION
%token TOK_IPTS_JUMP
%token TOK_IPTS_IN_INTERFACE
%token TOK_IPTS_OUT_INTERFACE
%token TOK_IPTS_FRAGMENT
%token TOK_IPTS_SET_COUNTERS
%token TOK_IPTS_MATCH

%token TOK_IPTS_DPORT

%token <u_str> TOK_IDENTIFIER
%token <u_str> TOK_OPTION

%token TOK_LSQUARE
%token TOK_RSQUARE
%token TOK_COLON
%token TOK_BANG
%token TOK_QUOTE
%token TOK_COMMIT
%token TOK_NEWLINE

%{
int ipts_print(FILE * f, int t, YYSTYPE v);
%}

%start ast

%%
ast: rule_list
{
    /* we expect parm to be already allocated, and that
     * it is of type (struct ast_s *) */
    ((struct ast_s *) parm)->list = $1;
}

rule_list: /* empty */
{
    $$ = NULL;
}
| rule_list rule TOK_NEWLINE
{
    $$ = malloc(sizeof(struct rule_list_s));
    $$->list = $1;
    $$->rule = $2;
}

rule: TOK_IPTS_TABLE TOK_IDENTIFIER
{
    $$ = malloc(sizeof(struct rule_s));
    $$->table = $2;
    $$->chain = NULL;
    $$->policy = NULL;
    $$->pkt_count = NULL;
    $$->option_list = NULL;
}
| TOK_IPTS_CHAIN TOK_IDENTIFIER TOK_IDENTIFIER pkt_count
{
    $$ = malloc(sizeof(struct rule_s));
    $$->table = NULL;
    $$->chain = $2;
    $$->policy = $3;
    $$->pkt_count = $4;
    $$->option_list = NULL;
}
| TOK_COMMIT
{
    $$ = NULL;
}
| pkt_count option_list
{
    $$ = malloc(sizeof(struct rule_s));
    $$->table = NULL;
    $$->chain = NULL;
    $$->policy = NULL;
    $$->pkt_count = $1;
    $$->option_list = $2;
}
| option_list
{
    $$ = malloc(sizeof(struct rule_s));
    $$->table = NULL;
    $$->chain = NULL;
    $$->policy = NULL;
    $$->pkt_count = NULL;
    $$->option_list = $1;
}

option_list: /* empty */
{
    $$ = NULL;
}
| option_list not_option
{
    $$ = malloc(sizeof(struct option_list_s));
    $$->option_list = $1;
    $$->not_option = $2;
}

not_option: TOK_BANG option
{
    $$ = malloc(sizeof(struct not_option_s));
    $$->neg = 1;
    $$->option = $2;
}
| option
{
    $$ = malloc(sizeof(struct not_option_s));
    $$->neg = 0;
    $$->option = $1;
}


option: TOK_OPTION not_identifier
{
    $$ = malloc(sizeof(struct option_s));
    $$->option = $1;
    $$->not_identifier = $2;
}
| TOK_OPTION
{
    $$ = malloc(sizeof(struct option_s));
    $$->option = $1;
    $$->not_identifier = NULL;
}

not_identifier: TOK_BANG identifier
{
    $$ = malloc(sizeof(struct not_identifier_s));
    $$->neg = 1;
    $$->identifier = $2;
}
| identifier
{
    $$ = malloc(sizeof(struct not_identifier_s));
    $$->neg = 0;
    $$->identifier = $1;
}

identifier: TOK_IDENTIFIER TOK_IDENTIFIER
{
    $$ = malloc(sizeof(struct identifier_s));
    $$->id1 = $1;
    $$->id2 = $2;
}
| TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER
{
    $$ = malloc(sizeof(struct identifier_s));
    asprintf(&($$->id1), "%s:%s", $1, $3);
    $$->id2 = NULL;
}
| TOK_QUOTE TOK_IDENTIFIER TOK_QUOTE
{
    $$ = malloc(sizeof(struct identifier_s));
    $$->id1 = $2;
    $$->id2 = NULL;

}
| TOK_IDENTIFIER
{
    $$ = malloc(sizeof(struct identifier_s));
    $$->id1 = $1;
    $$->id2 = NULL;
}

pkt_count: TOK_LSQUARE TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER TOK_RSQUARE
{
    $$ = malloc(sizeof(struct pkt_count_s));
    $$->in = $2;
    $$->out = $4;
}

%%
char * ipts_filename();
extern char * ipts_text;

void ipts_error(const char * s) {
    fprintf(stderr, "%s:%d: %s\n", ipts_filename(), ipts_lineno, s);
}

int ipts_print(FILE * f, int type, YYSTYPE v) {
    fprintf(f, "type=%d,spelling=\"%s\",loc=%p", type, ipts_text, &v);
    return 0;
}
