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
/* verbose error messages */
%error-verbose

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#define YYPARSE_PARAM parm

void ipts_error(const char * msg);
extern int ipts_lineno;
extern int ipts_lex(void);

#define YYPRINT(f, t, v) ipts_print(f, t, v)
%}
%debug

%union {
  struct table_list_s * u_table_list;
  struct table_s * u_table;
    struct rule_list_s * u_rule_list;
    struct rule_s * u_rule;
    struct option_list_s * u_option_list;
    struct not_option_s * u_not_option;
    struct option_s * u_option;
    struct in_interface_option_s * u_in_interface_option;
    struct jump_option_s * u_jump_option;
    struct destination_option_s * u_destination_option;
    struct protocol_option_s * u_protocol_option;
    struct match_option_s * u_match_option;
    struct dport_option_s * u_dport_option;
    struct to_ports_option_s * u_to_ports_option;
    struct source_option_s * u_source_option;
    struct out_interface_option_s * u_out_interface_option;
    struct to_source_option_s * u_to_source_option;

    struct not_identifier_s * u_not_identifier;
    struct identifier_s * u_identifier;
    struct pkt_count_s * u_pkt_count;
    char * u_str;
}
%type <u_table_list> table_list
%type <u_table> table
%type <u_rule_list> rule_list
%type <u_rule> rule
%type <u_option_list> rule_specification
%type <u_option_list> option_list
%type <u_not_option> not_option
%type <u_option> option
%type <u_to_source_option> to_source_option
%type <u_out_interface_option> out_interface_option
%type <u_source_option> source_option
%type <u_to_ports_option> to_ports_option
%type <u_dport_option> dport_option
%type <u_match_option> match_option
%type <u_protocol_option> protocol_option
%type <u_destination_option> destination_option
%type <u_jump_option> jump_option
%type <u_in_interface_option> in_interface_option
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
%token TOK_IPTS_SPORT

%token TOK_IPTS_SOURCE_PORTS
%token TOK_IPTS_DESTINATION_PORTS
%token TOK_IPTS_PORTS

%token TOK_IPTS_SYN
%token TOK_IPTS_TCP_FLAGS
%token TOK_IPTS_TCP_OPTION
%token TOK_IPTS_MSS

%token TOK_IPTS_STATE

%token TOK_IPTS_ICMP_TYPE

%token TOK_IPTS_TO_PORTS
%token TOK_IPTS_TO_SOURCE

%token <u_str> TOK_IDENTIFIER
%token <u_str> TOK_OPTION

%token TOK_LSQUARE
%token TOK_RSQUARE
%token TOK_COLON
%token TOK_BANG
%token TOK_QUOTE
%token TOK_COMMIT

%{
int ipts_print(FILE * f, int t, YYSTYPE v);
%}

%start ast

%%
ast: table_list
{
    /* we expect parm to be already allocated, and that
     * it is of type (struct ast_s *) */
    ((struct ast_s *) parm)->list = $1;
}

table_list: /* empty */
{
  $$ = NULL;
}
| table_list table
{
  $$ = malloc(sizeof(struct table_list_s));
  $$->list = $1;
  $$->table = $2;
}

table: TOK_IPTS_TABLE TOK_IDENTIFIER rule_list TOK_COMMIT
{
  $$ = malloc(sizeof(struct table_s));
  $$->name = $2;
  $$->rule_list = $3;
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

rule: TOK_IPTS_CHAIN TOK_IDENTIFIER TOK_IDENTIFIER pkt_count
{
    $$ = malloc(sizeof(struct rule_s));
    $$->chain = $2;
    $$->policy = $3;
    $$->pkt_count = $4;
    $$->option_list = NULL;
}
| pkt_count chain_command TOK_IDENTIFIER rule_specification
{
    $$ = malloc(sizeof(struct rule_s));
    $$->chain = $3;
    $$->policy = NULL;
    $$->pkt_count = $1;
    $$->option_list = $4;
}

chain_command: TOK_IPTS_CHAIN_APPEND

rule_specification: option_list
{
    $$ = $1;
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

option: in_interface_option
{
  $$ = malloc(sizeof(struct option_s));
  $$->in_interface_option = $1;
}
| jump_option
{
  $$ = malloc(sizeof(struct option_s));
  $$->jump_option = $1;
}
| destination_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->destination_option = $1;
}
| protocol_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->protocol_option = $1;
}
| match_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->match_option = $1;
}
| dport_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->dport_option = $1;
}
| to_ports_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->to_ports_option = $1;
}
| source_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->source_option = $1;
}
| out_interface_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->out_interface_option = $1;
}
| to_source_option
{
    $$ = malloc(sizeof(struct option_s));
    $$->to_source_option = $1;
}

to_source_option: TOK_IPTS_TO_SOURCE identifier
{
    $$ = malloc(sizeof(struct to_source_option_s));
    $$->identifier = $2;
};

out_interface_option: TOK_IPTS_OUT_INTERFACE not_identifier
{
    $$ = malloc(sizeof(struct out_interface_option_s));
    $$->not_identifier = $2;
}

source_option: TOK_IPTS_SOURCE not_identifier
{
    $$ = malloc(sizeof(struct source_option_s));
    $$->not_identifier = $2;
}

to_ports_option: TOK_IPTS_TO_PORTS identifier
{
    $$ = malloc(sizeof(struct to_ports_option_s));
    $$->identifier = $2;
}

dport_option: TOK_IPTS_DPORT identifier
{
    $$ = malloc(sizeof(struct dport_option_s));
    $$->identifier = $2;
}

match_option: TOK_IPTS_MATCH identifier
{
    $$ = malloc(sizeof(struct match_option_s));
    $$->identifier = $2;
}

protocol_option: TOK_IPTS_PROTOCOL identifier
{
    $$ = malloc(sizeof(struct protocol_option_s));
    $$->identifier = $2;
}

destination_option: TOK_IPTS_DESTINATION not_identifier
{
    $$ = malloc(sizeof(struct destination_option_s));
    $$->not_identifier = $2;
}

jump_option: TOK_IPTS_JUMP identifier
{
  $$ = malloc(sizeof(struct jump_option_s));
  $$->identifier = $2;
}

in_interface_option: TOK_IPTS_IN_INTERFACE not_identifier
{
  $$ = malloc(sizeof(struct in_interface_option_s));
  $$->not_identifier = $2;
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

void ipts_error(const char * msg) {
  fprintf(stderr, "%s:%d: %s\n", ipts_filename(), ipts_lineno, msg);
}

int ipts_print(FILE * f, int type, YYSTYPE v) {
    fprintf(f, "type=%d,spelling=\"%s\",loc=%p", type, ipts_text, &v);
    return 0;
}
