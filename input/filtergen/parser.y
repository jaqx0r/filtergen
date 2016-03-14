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

/* prepend all functions with filtergen_ to keep the namespace separate
 * from other parsers */
%name-prefix "filtergen_"
/* verbose parser errors */
%error-verbose
%locations
%parse-param {struct ast_s *ast}
%{
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input/filtergen/ast.h"
#include "input/sourcepos.h"

void filtergen_error(struct ast_s *ast, const char * s, ...);
extern int filtergen_lex(void);

#define YYPRINT(f, t, v) filtergen_print(f, t, v)
%}

%code requires {
typedef struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  struct sourcefile* srcfile;
} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1 /* alert the parser that we have our own definition */

# define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                 \
      if (N)                                                           \
        {                                                              \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;       \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;     \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;        \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;      \
          (Current).srcfile      = YYRHSLOC (Rhs, 1).srcfile;          \
        }                                                              \
      else                                                             \
        { /* empty RHS */                                              \
          (Current).first_line   = (Current).last_line   =             \
            YYRHSLOC (Rhs, 0).last_line;                               \
          (Current).first_column = (Current).last_column =             \
            YYRHSLOC (Rhs, 0).last_column;                             \
          (Current).srcfile  = NULL;                        /* new */  \
        }                                                              \
    while (0)
}
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
%token TOK_UNEXPECTED
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
%token TOK_BANG
%token TOK_COLON
%token TOK_STAR
%{
int filtergen_print(FILE * f, int t, YYSTYPE v);
struct sourceposition* make_sourcepos(YYLTYPE);
%}
%start ast
%%
ast: rule_list
	{
        /* we expect ast to be already allocated */
	    ast->list = $1;
        ast->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	;

rule:	  specifier_list TOK_SEMICOLON
	{
	    $$ = malloc(sizeof(struct rule_s));
	    $$->list = $1;
        $$->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	;

negated_specifier: specifier
	{
	    $$ = malloc(sizeof(struct negated_specifier_s));
	    $$->negated = 0;
	    $$->spec = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_BANG specifier
	{
	    $$ = malloc(sizeof(struct negated_specifier_s));
	    $$->negated = 1;
	    $$->spec = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	;

specifier: compound_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->compound = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| direction_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->direction = $1;
        $$->pos = make_sourcepos(yylloc);
	}		
	| target_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->target = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| host_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->host = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| port_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->port = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| protocol_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->protocol = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| icmptype_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->icmptype = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| option_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->option = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| chaingroup_specifier
	{
	    $$ = malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->chaingroup = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	;

direction_specifier: TOK_INPUT direction_argument_list
	{
	    $$ = malloc(sizeof(struct direction_specifier_s));
	    $$->type = TOK_INPUT;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_OUTPUT direction_argument_list
	{
	    $$ = malloc(sizeof(struct direction_specifier_s));
	    $$->type = TOK_OUTPUT;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
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
	| TOK_STAR
	{
		$$ = malloc(sizeof(struct direction_argument_list_s));
		$$->list = NULL;
		$$->arg = malloc(sizeof(struct direction_argument_s));
		$$->arg->direction = strdup("*");
        $$->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	;

direction_argument: TOK_IDENTIFIER
	{
	    $$ = malloc(sizeof(struct direction_argument_s));
	    $$->direction = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	;

target_specifier: TOK_ACCEPT
	{
	    $$ = malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_ACCEPT;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_REJECT
	{
	    $$ = malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_REJECT;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_DROP
	{
	    $$ = malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_DROP;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_MASQ
	{
	    $$ = malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_MASQ;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_PROXY
	{
	    $$ = malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_PROXY;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_REDIRECT
	{
	    $$ = malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_REDIRECT;
        $$->pos = make_sourcepos(yylloc);
	}
	;

host_specifier: TOK_SOURCE host_argument_list
	{
	    $$ = malloc(sizeof(struct host_specifier_s));
	    $$->type = TOK_SOURCE;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_DEST host_argument_list
	{
	    $$ = malloc(sizeof(struct host_specifier_s));
	    $$->type = TOK_DEST;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	;

host_argument: TOK_IDENTIFIER TOK_SLASH TOK_IDENTIFIER
	{
	    $$ = malloc(sizeof(struct host_argument_s));
	    $$->host = $1;
	    $$->mask = $3;
        $$->pos = make_sourcepos(yylloc);
	}
    | TOK_IDENTIFIER
    {
	    $$ = malloc(sizeof(struct host_argument_s));
	    $$->host = $1;
	    $$->mask = 0;
        $$->pos = make_sourcepos(yylloc);
	}
	;

port_specifier: TOK_SPORT port_argument_list
	{
	    $$ = malloc(sizeof(struct port_specifier_s));
	    $$->type = TOK_SPORT;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_DPORT port_argument_list
	{
	    $$ = malloc(sizeof(struct port_specifier_s));
	    $$->type = TOK_DPORT;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	| port_argument_list_ port_argument
	{
	    $$ = malloc(sizeof(struct port_argument_list_s));
	    $$->list = $1;
	    $$->arg = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	;

port_argument: TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER
	{
	    $$ = malloc(sizeof(struct port_argument_s));
	    $$->port_min = $1;
	    $$->port_max = $3;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_IDENTIFIER
	{
	    $$ = malloc(sizeof(struct port_argument_s));
	    $$->port_min = $1;
	    $$->port_max = NULL;
        $$->pos = make_sourcepos(yylloc);
	}
	;

protocol_specifier: TOK_PROTO protocol_argument_list
	{
	    $$ = malloc(sizeof(struct protocol_specifier_s));
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	;

protocol_argument: TOK_IDENTIFIER
	{
	    $$ = malloc(sizeof(struct protocol_argument_s));
	    $$->proto = strdup($1);
        $$->pos = make_sourcepos(yylloc);
	}
	;

icmptype_specifier: TOK_ICMPTYPE icmptype_argument_list
	{
	    $$ = malloc(sizeof(struct icmptype_specifier_s));
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
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
        $$->pos = make_sourcepos(yylloc);
	}
	;

icmptype_argument: TOK_IDENTIFIER
	{
	    $$ = malloc(sizeof(struct icmptype_argument_s));
	    $$->icmptype = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	;

option_specifier: TOK_LOCAL
	{
	    $$ = malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_LOCAL;
	    $$->logmsg = 0;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_FORWARD
	{
	    $$ = malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_FORWARD;
	    $$->logmsg = 0;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_ONEWAY
	{
	    $$ = malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_ONEWAY;
	    $$->logmsg = 0;
        $$->pos = make_sourcepos(yylloc);
	}
    | TOK_LOG TOK_TEXT TOK_IDENTIFIER
    {
	    $$ = malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_LOG;
	    $$->logmsg = $3;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_LOG
	{
	    $$ = malloc(sizeof(struct option_specifier_s));	
	    $$->type = TOK_LOG;
	    $$->logmsg = 0;
        $$->pos = make_sourcepos(yylloc);
	}
	;

compound_specifier: TOK_LCURLY subrule_list TOK_RCURLY
	{
	    $$ = malloc(sizeof(struct compound_specifier_s));
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	;

subrule_list: specifier_list
	{
	    $$ = malloc(sizeof(struct subrule_list_s));
	    $$->subrule_list = NULL;
	    $$->specifier_list = $1;
        $$->pos = make_sourcepos(yylloc);
	}
	| subrule_list TOK_SEMICOLON specifier_list
	{
	    $$ = malloc(sizeof(struct subrule_list_s));
	    $$->subrule_list = $1;
	    $$->specifier_list = $3;
        $$->pos = make_sourcepos(yylloc);
	}
	;

chaingroup_specifier: TOK_LSQUARE TOK_IDENTIFIER subrule_list TOK_RSQUARE
	{
	    $$ = malloc(sizeof(struct chaingroup_specifier_s));
	    $$->name = $2;
	    $$->list = $3;
        $$->pos = make_sourcepos(yylloc);
	}
	| TOK_LSQUARE subrule_list TOK_RSQUARE
	{
	    $$ = malloc(sizeof(struct chaingroup_specifier_s));
	    $$->name = NULL;
	    $$->list = $2;
        $$->pos = make_sourcepos(yylloc);
	}
	;

%%

extern char * filtergen_text;



void
 __attribute__((__format__ (__printf__, 2, 3)))
filtergen_error(struct ast_s __attribute__((__unused__)) *ast, const char *s, ...)  
{
    va_list ap;
    va_start(ap, s);

    if (yylloc.first_line) {
        fprintf(stderr, "%s:%d.%d-%d.%d: error: ", yylloc.srcfile->pathname, yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column);
    }
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

void 
 __attribute__((__format__ (__printf__, 1, 2)))
filtergen_warn(const char *s, ...) {
    va_list ap;
    va_start(ap, s);

    if (yylloc.first_line) {
        fprintf(stderr, "%s:%d.%d-%d.%d: warning: ", yylloc.srcfile->pathname, yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column);
    }
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

void
 __attribute__((__format__ (__printf__, 3,4)))
 lyyerror(YYLTYPE t, struct ast_s __attribute__((__unused__)) *ast, char *s, ...) {
    va_list ap;
    va_start(ap, s);

    if (t.first_line) {
        fprintf(stderr, "%s:%d.%d-%d.%d: error: ", t.srcfile->pathname, t.first_line, t.first_column, t.last_line, t.last_column);
    }
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

int filtergen_print(FILE * f, int type, YYSTYPE v) {
    fprintf(f, "%d:\"%s\":%p", type, filtergen_text, (void *) &v);
	return 0;
}

struct sourceposition *make_sourcepos(YYLTYPE loc) {
  struct sourceposition *pos;
  pos = malloc(sizeof(struct sourceposition));

  pos->first_line = loc.first_line;
  pos->first_column = loc.first_column;
  pos->last_line = loc.last_line;
  pos->last_column = loc.last_column;
  pos->filename = loc.srcfile->pathname;
  return pos;
}
