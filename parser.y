/* parser for filtergen
 *
 * Copyright (c) 2003 Jamie Wilkinson <jaq@spacepants.org>
 */

%{
#include <stdio.h>
#include <stdlib.h>
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
	char * u_str;
	int u_int;
}
%type <u_rule_list> rule_list
%type <u_rule> rule

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
%token <u_str> TOK_STRINGLITERAL
%token <u_str> TOK_TEXT
%token <u_str> TOK_IDENTIFIER
%token <u_int> TOK_NUMBER
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
		((struct ast_s *) parm)->rule_list = $1;
	}

rule_list: /* empty */
	{
		$$ = NULL;
	}
	| rule_list rule
	{
		$$ = malloc(sizeof(struct rule_list_s));
		$$->rule_list = $1;
		$$->rule = $2;
	}
	;

rule:	  specifier_list TOK_SEMICOLON
	{
		$$ = malloc(sizeof(struct rule_s));
	}
	;

specifier_list: /* empty */
	| specifier_list negated_specifier
	;

specifier: compound_specifier
	| direction_specifier
	| target_specifier
	| host_specifier
	| port_specifier
	| protocol_specifier
	| icmptype_specifier
	| routing_specifier
	| chaingroup_specifier
	;

negated_specifier: specifier
	| TOK_BANG specifier
	;

direction_specifier: TOK_INPUT direction_argument
	| TOK_OUTPUT direction_argument
	;

direction_argument: compound_direction_argument
	| simple_direction_argument
	;

compound_direction_argument: TOK_LCURLY direction_argument_list TOK_RCURLY
	;

direction_argument_list: simple_direction_argument
	| direction_argument_list simple_direction_argument
	;

simple_direction_argument: TOK_IDENTIFIER
	;

target_specifier: TOK_ACCEPT
	| TOK_REJECT
	| TOK_DROP
	| TOK_MASQ
	| TOK_PROXY
	| TOK_REDIRECT
	| log_target_specifier
	;

log_target_specifier: TOK_LOG
	| TOK_LOG TOK_TEXT log_text_argument
	;

host_specifier: TOK_SOURCE host_argument
	| TOK_DEST host_argument
	;

host_argument: compound_host_argument
	| simple_host_argument
	;

compound_host_argument: TOK_LCURLY host_argument_list TOK_RCURLY
	;

host_argument_list: simple_host_argument
	| host_argument_list simple_host_argument
	;

simple_host_argument: host_part
	| host_part TOK_SLASH netmask_part
	;

port_specifier: TOK_SPORT port_argument
	| TOK_DPORT port_argument
	;

port_argument: compound_port_argument
	| simple_port_argument
	;

compound_port_argument: TOK_LCURLY port_argument_list TOK_RCURLY
	;

port_argument_list: simple_port_argument
	| port_argument_list simple_port_argument
	;

simple_port_argument: port_range
	| port_single
	;

protocol_specifier: TOK_PROTO protocol_argument
	;

protocol_argument: compound_protocol_argument
	| simple_protocol_argument
	;

compound_protocol_argument: TOK_LCURLY protocol_argument_list TOK_RCURLY
	;

protocol_argument_list: simple_protocol_argument
	| protocol_argument_list simple_protocol_argument
	;

simple_protocol_argument: TOK_IDENTIFIER
	;

icmptype_specifier: TOK_ICMPTYPE icmptype_argument;
	;

icmptype_argument: compound_icmptype_argument
	| simple_icmptype_argument
	;

compound_icmptype_argument: TOK_LCURLY icmptype_argument_list TOK_RCURLY
	;

icmptype_argument_list: simple_icmptype_argument
	| icmptype_argument_list simple_icmptype_argument
	;

simple_icmptype_argument: TOK_IDENTIFIER
	;

routing_specifier: TOK_LOCAL
	| TOK_FORWARD
	;

compound_specifier: TOK_LCURLY subrule_list TOK_RCURLY
	;

subrule_list: specifier_list
	| subrule_list TOK_SEMICOLON specifier_list
	;

log_text_argument: TOK_STRINGLITERAL
	| TOK_IDENTIFIER
	;

chaingroup_specifier: TOK_LSQUARE TOK_STRINGLITERAL subrule_list TOK_RSQUARE
	| TOK_LSQUARE subrule_list TOK_RSQUARE
	;

host_part: TOK_IDENTIFIER
	| TOK_NUMBER TOK_DOT TOK_NUMBER TOK_DOT TOK_NUMBER TOK_DOT TOK_NUMBER
	;

netmask_part: TOK_NUMBER
	| TOK_NUMBER TOK_DOT TOK_NUMBER
	| TOK_NUMBER TOK_DOT TOK_NUMBER TOK_DOT TOK_NUMBER
	| TOK_NUMBER TOK_DOT TOK_NUMBER TOK_DOT TOK_NUMBER TOK_DOT TOK_NUMBER
	;

port_range: port_single TOK_COLON port_single
	;

port_single: TOK_IDENTIFIER
	| TOK_NUMBER
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

void yyrestart(FILE *);

int filter_fopen(const char *filename)
{
	FILE *file;

	if(filename) {
		/* XXX - make more effort to find file */
		if(!(file = fopen(filename, "r"))) {
		    /*parse_err("can't open file \"%s\"", filename); */
			return -1;
		}
	} else {
		/* Standard input */
		file = stdin;
	}
	yyrestart(file);
	/*

	state.incldepth++;
	yyin = file;
	yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
	curinc.bufferstat = YY_CURRENT_BUFFER;
	curinc.filename = filename ? strdup(filename) : NULL;
	curinc.lineno = 1;
	*/
	return 0;
}
