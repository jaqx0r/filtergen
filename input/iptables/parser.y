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

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#define YYPARSE_PARAM parm

void yyerror(const char *);
extern int yylex(void);

#define YYPRINT(f, t, v) yyprint(f, t, v)
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
%token TOK_TABLE
%token TOK_CHAIN
%token <u_str> TOK_OPTION
%token <u_str> TOK_IDENTIFIER
%token TOK_LSQUARE
%token TOK_RSQUARE
%token TOK_COLON
%token TOK_BANG
%token TOK_QUOTE
%token TOK_COMMIT
%token TOK_NEWLINE

%{
int yyprint(FILE * f, int t, YYSTYPE v);
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

rule: TOK_TABLE TOK_IDENTIFIER
{
    $$ = NULL;
}
| TOK_CHAIN TOK_IDENTIFIER TOK_IDENTIFIER pkt_count
{
    $$ = NULL;
}
| TOK_COMMIT
{
    $$ = NULL;
}
| pkt_count option_list
{
    $$ = NULL;
}
| option_list
{
    $$ = NULL;
}

option_list: /* empty */
{
    $$ = NULL;
}
| option_list not_option
{
    $$ = NULL;
}

not_option: TOK_BANG option
{
    $$ = NULL;
}
| option
{
    $$ = NULL;
}

option: TOK_OPTION not_identifier
{
    $$ = NULL;
}

not_identifier: TOK_BANG identifier
{
    $$ = NULL;
}
| identifier
{
    $$ = NULL;
}

identifier: TOK_IDENTIFIER TOK_IDENTIFIER
{
    $$ = NULL;
}
| TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER
{
    $$ = NULL;
}
| TOK_QUOTE TOK_IDENTIFIER TOK_QUOTE
{
    $$ = NULL;
}
| TOK_IDENTIFIER
{
    $$ = NULL;
}

pkt_count: TOK_LSQUARE TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER TOK_RSQUARE
{
    $$ = NULL;
}

%%
char * filename();
long int lineno();
extern char * yytext;

void yyerror(const char * s) {
    fprintf(stderr, "%s:%ld: %s\n", filename(), lineno(), s);
}

int yyprint(FILE * f, int type, YYSTYPE v) {
    fprintf(f, "type=%d,spelling=\"%s\",loc=%p", type, yytext, &v);
    return 0;
}
