/* parser for filtergen
 *
 * Copyright (c) 2003 Jamie Wilkinson <jaq@spacepants.org>
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define YYPARSE_PARAM parm

void yyerror(char * s);
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
	struct direction_argument_s * u_direction_argument;
	struct compound_direction_argument_s * u_compound_direction_argument;
	struct direction_argument_list_s * u_direction_argument_list;
	struct simple_direction_argument_s * u_simple_direction_argument;
	struct target_specifier_s * u_target_specifier;
	struct log_target_specifier_s * u_log_target_specifier;
	struct host_specifier_s * u_host_specifier;
	struct host_argument_s * u_host_argument;
	struct compound_host_argument_s * u_compound_host_argument;
	struct host_argument_list_s * u_host_argument_list;
	struct simple_host_argument_s * u_simple_host_argument;
	struct port_specifier_s * u_port_specifier;
	struct port_argument_s * u_port_argument;
	struct compound_port_argument_s * u_compound_port_argument;
	struct port_argument_list_s * u_port_argument_list;
	struct simple_port_argument_s * u_simple_port_argument;
	struct protocol_specifier_s * u_protocol_specifier;
	struct protocol_argument_s * u_protocol_argument;
	struct compound_protocol_argument_s * u_compound_protocol_argument;
	struct protocol_argument_list_s * u_protocol_argument_list;
	struct simple_protocol_argument_s * u_simple_protocol_argument;
	struct icmptype_specifier_s * u_icmptype_specifier;
	struct icmptype_argument_s * u_icmptype_argument;
	struct compound_icmptype_argument_s * u_compound_icmptype_argument;
	struct icmptype_argument_list_s * u_icmptype_argument_list;
	struct simple_icmptype_argument_s * u_simple_icmptype_argument;
	struct routing_specifier_s * u_routing_specifier;
	struct compound_specifier_s * u_compound_specifier;
	struct log_text_argument_s * u_log_text_argument;
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
%type <u_direction_argument> direction_argument
%type <u_compound_direction_argument> compound_direction_argument
%type <u_direction_argument_list> direction_argument_list
%type <u_simple_direction_argument> simple_direction_argument
%type <u_target_specifier> target_specifier
%type <u_host_specifier> host_specifier
%type <u_host_argument> host_argument
%type <u_compound_host_argument> compound_host_argument
%type <u_host_argument_list> host_argument_list
%type <u_simple_host_argument> simple_host_argument
%type <u_port_specifier> port_specifier
%type <u_port_argument> port_argument
%type <u_compound_port_argument> compound_port_argument
%type <u_port_argument_list> port_argument_list
%type <u_simple_port_argument> simple_port_argument
%type <u_protocol_specifier> protocol_specifier
%type <u_protocol_argument> protocol_argument
%type <u_compound_protocol_argument> compound_protocol_argument
%type <u_protocol_argument_list> protocol_argument_list
%type <u_simple_protocol_argument> simple_protocol_argument
%type <u_icmptype_specifier> icmptype_specifier
%type <u_icmptype_argument> icmptype_argument
%type <u_compound_icmptype_argument> compound_icmptype_argument
%type <u_icmptype_argument_list> icmptype_argument_list
%type <u_simple_icmptype_argument> simple_icmptype_argument
%type <u_routing_specifier> routing_specifier
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
	| routing_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->routing = $1;
	}
	| chaingroup_specifier
	{
		$$ = malloc(sizeof(struct specifier_s));
		memset($$, 0, sizeof(struct specifier_s));
		$$->chaingroup = $1;
	}
	;

direction_specifier: TOK_INPUT direction_argument
	{
		$$ = malloc(sizeof(struct direction_specifier_s));
		$$->type = TOK_INPUT;
		$$->arg = $2;
	}
	| TOK_OUTPUT direction_argument
	{
		$$ = malloc(sizeof(struct direction_specifier_s));
		$$->type = TOK_OUTPUT;
		$$->arg = $2;
	}
	;

direction_argument: compound_direction_argument
	{
		$$ = malloc(sizeof(struct direction_argument_s));
		$$->compound = $1;
		$$->simple = NULL;
	}
	| simple_direction_argument
	{
		$$ = malloc(sizeof(struct direction_argument_s));
		$$->compound = NULL;
		$$->simple = $1;
	}
	;

compound_direction_argument: TOK_LCURLY direction_argument_list TOK_RCURLY
	{
		$$ = malloc(sizeof(struct compound_direction_argument_s));
		$$->list = $2;
	}
	;

direction_argument_list: /* empty */
	{
		$$ = NULL;
	}
	| direction_argument_list simple_direction_argument
	{
		$$ = malloc(sizeof(struct direction_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

simple_direction_argument: TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct simple_direction_argument_s));
		$$->identifier = $1;
	}
	;

target_specifier: TOK_ACCEPT
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_ACCEPT;
		$$->logtext = NULL;
	}
	| TOK_REJECT
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_REJECT;
		$$->logtext = NULL;
	}
	| TOK_DROP
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_DROP;
		$$->logtext = NULL;
	}
	| TOK_MASQ
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_MASQ;
		$$->logtext = NULL;
	}
	| TOK_PROXY
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_PROXY;
		$$->logtext = NULL;
	}
	| TOK_REDIRECT
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_REDIRECT;
		$$->logtext = NULL;
	}
	| TOK_LOG
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_LOG;
		$$->logtext = NULL;
	}
	| TOK_LOG TOK_TEXT TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct target_specifier_s));
		$$->type = TOK_LOG;
		$$->logtext = $3;
	}
	;

host_specifier: TOK_SOURCE host_argument
	{
		$$ = malloc(sizeof(struct host_specifier_s));
		$$->type = TOK_SOURCE;
		$$->arg = $2;
	}
	| TOK_DEST host_argument
	{
		$$ = malloc(sizeof(struct host_specifier_s));
		$$->type = TOK_DEST;
		$$->arg = $2;
	}
	;

host_argument: compound_host_argument
	{
		$$ = malloc(sizeof(struct host_argument_s));
		$$->compound = $1;
		$$->simple = NULL;
	}
	| simple_host_argument
	{
		$$ = malloc(sizeof(struct host_argument_s));
		$$->compound = NULL;
		$$->simple = $1;
	}
	;

compound_host_argument: TOK_LCURLY host_argument_list TOK_RCURLY
	{
		$$ = malloc(sizeof(struct compound_host_argument_s));
		$$->list = $2;
	}
	;

host_argument_list: /* empty */
	{
		$$ = NULL;
	}
	| host_argument_list simple_host_argument
	{
		$$ = malloc(sizeof(struct host_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

simple_host_argument: TOK_IDENTIFIER TOK_SLASH TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct simple_host_argument_s));
		$$->host = $1;
		$$->mask = $3;
	}
        | TOK_IDENTIFIER
        {
		$$ = malloc(sizeof(struct simple_host_argument_s));
		$$->host = $1;
                $$->mask = 0;
	}
	;

port_specifier: TOK_SPORT port_argument
	{
		$$ = malloc(sizeof(struct port_specifier_s));
		$$->type = TOK_SPORT;
		$$->arg = $2;
	}
	| TOK_DPORT port_argument
	{
		$$ = malloc(sizeof(struct port_specifier_s));
		$$->type = TOK_DPORT;
		$$->arg = $2;
	}
	;

port_argument: compound_port_argument
	{
		$$ = malloc(sizeof(struct port_argument_s));
		$$->compound = $1;
		$$->simple = NULL;
	}
	| simple_port_argument
	{
		$$ = malloc(sizeof(struct port_argument_s));
		$$->compound = NULL;
		$$->simple = $1;
	}
	;

compound_port_argument: TOK_LCURLY port_argument_list TOK_RCURLY
	{
		$$ = malloc(sizeof(struct compound_port_argument_s));
		$$->list = $2;
	}
	;

port_argument_list: simple_port_argument
	{
		$$ = malloc(sizeof(struct port_argument_list_s));
		$$->list = NULL;
		$$->arg = $1;
	}
	| port_argument_list simple_port_argument
	{
		$$ = malloc(sizeof(struct port_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

simple_port_argument: TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct simple_port_argument_s));
		$$->port_min = $1;
		$$->port_max = $3;
	}
	| TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct simple_port_argument_s));
		$$->port_min = $1;
		$$->port_max = NULL;
	}
	;

protocol_specifier: TOK_PROTO protocol_argument
	{
		$$ = malloc(sizeof(struct protocol_specifier_s));
		$$->arg = $2;
	}
	;

protocol_argument: compound_protocol_argument
	{
		$$ = malloc(sizeof(struct protocol_argument_s));
		$$->compound = $1;
		$$->simple = NULL;
	}
	| simple_protocol_argument
	{
		$$ = malloc(sizeof(struct protocol_argument_s));
		$$->compound = NULL;
		$$->simple = $1;
	}
	;

compound_protocol_argument: TOK_LCURLY protocol_argument_list TOK_RCURLY
	{
		$$ = malloc(sizeof(struct compound_protocol_argument_s));
		$$->list = $2;
	}
	;

protocol_argument_list: /* empty */
	{
		$$ = NULL;
	}
	| protocol_argument_list simple_protocol_argument
	{
		$$ = malloc(sizeof(struct protocol_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

simple_protocol_argument: TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct simple_protocol_argument_s));
		$$->proto = strdup($1);
	}
	;

icmptype_specifier: TOK_ICMPTYPE icmptype_argument
	{
		$$ = malloc(sizeof(struct icmptype_specifier_s));
		$$->arg = $2;
	}
	;

icmptype_argument: compound_icmptype_argument
	{
		$$ = malloc(sizeof(struct icmptype_argument_s));
		$$->compound = $1;
		$$->simple = NULL;
	}
	| simple_icmptype_argument
	{
		$$ = malloc(sizeof(struct icmptype_argument_s));
		$$->compound = NULL;
		$$->simple = $1;
	}
	;

compound_icmptype_argument: TOK_LCURLY icmptype_argument_list TOK_RCURLY
	{
		$$ = malloc(sizeof(struct compound_icmptype_argument_s));
		$$->list = $2;
	}
	;

icmptype_argument_list: /* empty */
	{
		$$ = NULL;
	}
	| icmptype_argument_list simple_icmptype_argument
	{
		$$ = malloc(sizeof(struct icmptype_argument_list_s));
		$$->list = $1;
		$$->arg = $2;
	}
	;

simple_icmptype_argument: TOK_IDENTIFIER
	{
		$$ = malloc(sizeof(struct simple_icmptype_argument_s));
		$$->identifier = strdup($1);
	}
	;

routing_specifier: TOK_LOCAL
	{
		$$ = malloc(sizeof(struct routing_specifier_s));
		$$->type = TOK_LOCAL;
	}
	| TOK_FORWARD
	{
		$$ = malloc(sizeof(struct routing_specifier_s));
		$$->type = TOK_FORWARD;
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
		$$->name = strdup($2);
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

void yyerror(char * s) {
	fprintf(stderr, "%s:%ld: %s\n", filename(), lineno(), s);
}

int yyprint(FILE * f, int type, YYSTYPE v) {
	fprintf(f, "%s", yytext);
	return 0;
}
