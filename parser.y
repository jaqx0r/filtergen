/* parser for filtergen
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
#include <string.h>
#include "ast.h"

#define YYPARSE_PARAM parm

void yyerror(const char * s);
extern int yylex(void);

#define YYPRINT(f, t, v) yyprint(f, t, v)
%}
%debug

%union {
	struct rule_list_s * u_rule_list;
	struct rule_s * u_rule;
	struct specifier_list_s * u_specifier_list;
	struct negated_specifier_s * u_negated_specifier;
	struct specifier_s * u_specifier;
	struct direction_specifier_s * u_direction_specifier;
	struct direction_argument_list_s * u_direction_argument_list;
	struct direction_argument_s * u_direction_argument;
	struct target_specifier_s * u_target_specifier;
	struct log_target_specifier_s * u_log_target_specifier;
	struct host_specifier_s * u_host_specifier;
	struct host_argument_list_s * u_host_argument_list;
	struct host_argument_s * u_host_argument;
	struct port_specifier_s * u_port_specifier;
	struct port_argument_list_s * u_port_argument_list;
	struct port_argument_s * u_port_argument;
	struct protocol_specifier_s * u_protocol_specifier;
	struct protocol_argument_list_s * u_protocol_argument_list;
	struct protocol_argument_s * u_protocol_argument;
	struct icmptype_specifier_s * u_icmptype_specifier;
	struct icmptype_argument_list_s * u_icmptype_argument_list;
	struct icmptype_argument_s * u_icmptype_argument;
	struct option_specifier_s * u_option_specifier;
	struct compound_specifier_s * u_compound_specifier;
	struct chaingroup_specifier_s * u_chaingroup_specifier;
	struct subrule_list_s * u_subrule_list;
	char * u_str;
}
%type <u_rule_list> rule_list
%type <u_rule> rule
%type <u_specifier_list> specifier_list
%type <u_negated_specifier> negated_specifier
%type <u_specifier> specifier
%type <u_direction_specifier> direction_specifier
%type <u_direction_argument_list> direction_argument_list
%type <u_direction_argument_list> direction_argument_list_
%type <u_direction_argument> direction_argument
%type <u_target_specifier> target_specifier
%type <u_host_specifier> host_specifier
%type <u_host_argument_list> host_argument_list
%type <u_host_argument_list> host_argument_list_
%type <u_host_argument> host_argument
%type <u_port_specifier> port_specifier
%type <u_port_argument_list> port_argument_list
%type <u_port_argument_list> port_argument_list_
%type <u_port_argument> port_argument
%type <u_protocol_specifier> protocol_specifier
%type <u_protocol_argument_list> protocol_argument_list
%type <u_protocol_argument_list> protocol_argument_list_
%type <u_protocol_argument> protocol_argument
%type <u_icmptype_specifier> icmptype_specifier
%type <u_icmptype_argument_list> icmptype_argument_list
%type <u_icmptype_argument_list> icmptype_argument_list_
%type <u_icmptype_argument> icmptype_argument
%type <u_option_specifier> option_specifier
%type <u_compound_specifier> compound_specifier
%type <u_chaingroup_specifier> chaingroup_specifier
%type <u_subrule_list> subrule_list

%defines
%token TOK_ACCEPT
%token TOK_DEST
%token TOK_DPORT
%token TOK_DROP
%token TOK_FORWARD
%token TOK_ICMPTYPE
%token TOK_INPUT
%token TOK_LCURLY
%token TOK_LOCAL
%token TOK_LOG
%token TOK_LSQUARE
%token TOK_MASQ
%token TOK_ONEWAY
%token TOK_OUTPUT
%token TOK_PROTO
%token TOK_PROXY
%token TOK_RCURLY
%token TOK_REDIRECT
%token TOK_REJECT
%token TOK_RSQUARE
%token TOK_SEMICOLON
%token TOK_SOURCE
%token TOK_SPORT
%token TOK_TEXT
%token <u_str> TOK_IDENTIFIER
%token TOK_DOT
%token TOK_SLASH
%token TOK_ERR
%token TOK_BANG
%token TOK_COLON
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
	| rule_list rule
	{
		$$ = malloc(sizeof(struct rule_list_s));
		$$->list = $1;
		$$->rule = $2;
	}
	;

rule:	  specifier_list TOK_SEMICOLON
	{
		$$ = malloc(sizeof(struct rule_s));
		$$->list = $1;
	}
	;

specifier_list: /* empty */
	{
		$$ = NULL;
	}
	| specifier_list negated_specifier
	{
		$$ = malloc(sizeof(struct specifier_list_s));
		$$->list = $1;
		$$->spec = $2;
	}
	;

negated_specifier: specifier
	{
		$$ = malloc(sizeof(struct negated_specifier_s));
		$$->negated = 0;
		$$->spec = $1;
	}
	| TOK_BANG specifier
	{
		$$ = malloc(sizeof(struct negated_specifier_s));
		$$->negated = 1;
		$$->spec = $2;
	}
	;

specifier: compound_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->compound = $1;
	}
	| direction_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->direction = $1;
	}		
	| target_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->target = $1;
	}
	| host_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->host = $1;
	}
	| port_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->port = $1;
	}
	| protocol_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->protocol = $1;
	}
	| icmptype_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->icmptype = $1;
	}
	| option_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->option = $1;
	}
	| chaingroup_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->chaingroup = $1;
	}
	;

direction_specifier: TOK_INPUT direction_argument_list
	{
		$$ = malloc(sizeof(struct direction_specifier_s));
		$$->type = TOK_INPUT;
		$$->list = $2;
	}
	| TOK_OUTPUT direction_argument_list
	{
		$$ = malloc(sizeof(struct direction_specifier_s));
		$$->type = TOK_OUTPUT;
		$$->list = $2;
	}
	;

direction_argument_list: direction_argument_list_
	{
		$$ = $1;
	}
	| TOK_LCURLY direction_argument_list_ TOK_RCURLY
	{
		$$ = $2;
	}
	;

direction_argument_list_: /* empty */
	{
		$$ = NULL;
	}
	| direction_argument_list_ direction_argument
	{
		$$ = malloc(sizeof(struct direction_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

direction_argument: TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct direction_argument_s));
		$$->direction = $1;
	}
	;

target_specifier: TOK_ACCEPT
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_ACCEPT;
	}
	| TOK_REJECT
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_REJECT;
	}
	| TOK_DROP
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_DROP;
	}
	| TOK_MASQ
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_MASQ;
	}
	| TOK_PROXY
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_PROXY;
	}
	| TOK_REDIRECT
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_REDIRECT;
	}
	;

host_specifier: TOK_SOURCE host_argument_list
	{
		$$ = malloc(sizeof(struct host_specifier_s));
		$$->type = TOK_SOURCE;
		$$->list = $2;
	}
	| TOK_DEST host_argument_list
	{
		$$ = malloc(sizeof(struct host_specifier_s));
		$$->type = TOK_DEST;
		$$->list = $2;
	}
	;

host_argument_list: host_argument_list_
	{
		$$ = $1;
	}
	| TOK_LCURLY host_argument_list_ TOK_RCURLY
	{
		$$ = $2;
	}
	;

host_argument_list_: /* empty */
	{
		$$ = NULL;
	}
	| host_argument_list_ host_argument
	{
		$$ = malloc(sizeof(struct host_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

host_argument: TOK_IDENTIFIER TOK_SLASH TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct host_argument_s));
		$$->host = $1;
		$$->mask = $3;
	}
        | TOK_IDENTIFIER
        {
		$$ = malloc(sizeof(struct host_argument_s));
		$$->host = $1;
                $$->mask = 0;
	}
	;

port_specifier: TOK_SPORT port_argument_list
	{
		$$ = malloc(sizeof(struct port_specifier_s));
		$$->type = TOK_SPORT;
		$$->list = $2;
	}
	| TOK_DPORT port_argument_list
	{
		$$ = malloc(sizeof(struct port_specifier_s));
		$$->type = TOK_DPORT;
		$$->list = $2;
	}
	;

port_argument_list: port_argument_list_
	{
		$$ = $1;
	}
	| TOK_LCURLY port_argument_list TOK_RCURLY
	{
		$$ = $2;
	}

port_argument_list_: port_argument
	{
		$$ = malloc(sizeof(struct port_argument_list_s));
		$$->list = NULL;
		$$->arg = $1;
	}
	| port_argument_list_ port_argument
	{
		$$ = malloc(sizeof(struct port_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

port_argument: TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct port_argument_s));
		$$->port_min = $1;
		$$->port_max = $3;
	}
	| TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct port_argument_s));
		$$->port_min = $1;
		$$->port_max = NULL;
	}
	;

protocol_specifier: TOK_PROTO protocol_argument_list
	{
		$$ = malloc(sizeof(struct protocol_specifier_s));
		$$->list = $2;
	}
	;

protocol_argument_list: protocol_argument_list_
	{
		$$ = $1;
	}
	| TOK_LCURLY protocol_argument_list_ TOK_RCURLY
	{
		$$ = $2;
	}
	;

protocol_argument_list_: /* empty */
	{
		$$ = NULL;
	}
	| protocol_argument_list_ protocol_argument
	{
		$$ = malloc(sizeof(struct protocol_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

protocol_argument: TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct protocol_argument_s));
		$$->proto = strdup($1);
	}
	;

icmptype_specifier: TOK_ICMPTYPE icmptype_argument_list
	{
		$$ = malloc(sizeof(struct icmptype_specifier_s));
		$$->list = $2;
	}
	;

icmptype_argument_list: icmptype_argument_list_
	{
		$$ = $1;
	}
	| TOK_LCURLY icmptype_argument_list_ TOK_RCURLY
	{
		$$ = $2;
	}
	;

icmptype_argument_list_: /* empty */
	{
		$$ = NULL;
	}
	| icmptype_argument_list_ icmptype_argument
	{
		$$ = malloc(sizeof(struct icmptype_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

icmptype_argument: TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct icmptype_argument_s));
		$$->icmptype = $1;
	}
	;

option_specifier: TOK_LOCAL
	{
		$$ = malloc(sizeof(struct option_specifier_s));
		$$->type = TOK_LOCAL;
		$$->logmsg = 0;
	}
	| TOK_FORWARD
	{
		$$ = malloc(sizeof(struct option_specifier_s));
		$$->type = TOK_FORWARD;
		$$->logmsg = 0;
	}
	| TOK_ONEWAY
	{
		$$ = malloc(sizeof(struct option_specifier_s));
		$$->type = TOK_ONEWAY;
		$$->logmsg = 0;
	}
        | TOK_LOG TOK_TEXT TOK_IDENTIFIER
        {
                $$ = malloc(sizeof(struct option_specifier_s));
		$$->type = TOK_LOG;
		$$->logmsg = $3;
	}
	| TOK_LOG
	{
                $$ = malloc(sizeof(struct option_specifier_s));	
		$$->type = TOK_LOG;
		$$->logmsg = 0;
	}
	;

compound_specifier: TOK_LCURLY subrule_list TOK_RCURLY
	{
		$$ = malloc(sizeof(struct compound_specifier_s));
		$$->list = $2;
	}
	;

subrule_list: specifier_list
	{
		$$ = malloc(sizeof(struct subrule_list_s));
		$$->subrule_list = NULL;
		$$->specifier_list = $1;
	}
	| subrule_list TOK_SEMICOLON specifier_list
	{
		$$ = malloc(sizeof(struct subrule_list_s));
		$$->subrule_list = $1;
		$$->specifier_list = $3;
	}
	;

chaingroup_specifier: TOK_LSQUARE TOK_IDENTIFIER subrule_list TOK_RSQUARE
	{
		$$ = malloc(sizeof(struct chaingroup_specifier_s));
		$$->name = $2;
		$$->list = $3;
	}
	| TOK_LSQUARE subrule_list TOK_RSQUARE
	{
		$$ = malloc(sizeof(struct chaingroup_specifier_s));
		$$->name = NULL;
		$$->list = $2;
	}
	;

%%
char * filename();
long int lineno();
extern char * yytext;

void yyerror(const char * s) {
	fprintf(stderr, "%s:%ld: %s\n", filename(), lineno(), s);
}

int yyprint(FILE * f, int type, YYSTYPE v) {
	fprintf(f, "%d:\"%s\":%p", type, yytext, &v);
	return 0;
}
