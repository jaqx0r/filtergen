/* parser for iptables-save format    -*- C++ -*-
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

/* prepend all functions with ipts_ to keep the namespace
 * separate from other parsers */
/*%name-prefix="ipts_"*/

%skeleton "lalr1.cc"
%define "parser_class_name" "ipts_parser"
%defines
%{
#include <string>
#include "ast.h"
#include "driver.h"
%}

%parse-param { ipts_driver & driver }
%lex-param { ipts_driver & driver }

%locations

%initial-action
{
	// Initialise the initial location
	@$.begin.filename = @$.end.filename = &driver.filename;
};

%debug
%error-verbose

%union
{
	struct ast_s * u_ast;
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
    struct state_option_s * u_state_option;
    struct limit_option_s * u_limit_option;
    struct log_prefix_option_s * u_log_prefix_option;
    struct sport_option_s * u_sport_option;
    struct uid_owner_option_s * u_uid_owner_option;
    struct tcp_flags_option_s * u_tcp_flags_option;
    struct reject_with_option_s * u_reject_with_option;
    struct icmp_type_option_s * u_icmp_type_option;
    struct fragment_option_s * u_fragment_option;
    struct clamp_mss_to_pmtu_option_s * u_clamp_mss_to_pmtu_option;
    struct helper_option_s * u_helper_option;
    struct syn_option_s * u_syn_option;

    struct not_range_s * u_not_range;
    struct range_s * u_range;
    struct not_identifier_s * u_not_identifier;
    struct identifier_s * u_identifier;
    struct pkt_count_s * u_pkt_count;
    char * u_str;
};
%type <u_ast> ast
%type <u_table_list> table_list
%type <u_table> table
%type <u_rule_list> rule_list
%type <u_rule> rule
%type <u_option_list> rule_specification
%type <u_option_list> option_list
%type <u_not_option> not_option
%type <u_option> option

%type <u_syn_option> syn_option
%type <u_helper_option> helper_option
%type <u_clamp_mss_to_pmtu_option> clamp_mss_to_pmtu_option
%type <u_fragment_option> fragment_option
%type <u_icmp_type_option> icmp_type_option
%type <u_reject_with_option> reject_with_option
%type <u_tcp_flags_option> tcp_flags_option
%type <u_uid_owner_option> uid_owner_option
%type <u_sport_option> sport_option
%type <u_log_prefix_option> log_prefix_option
%type <u_limit_option> limit_option
%type <u_state_option> state_option
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

%type <u_not_range> not_range
%type <u_range> range
%type <u_not_identifier> not_identifier
%type <u_identifier> identifier
%type <u_pkt_count> opt_pkt_count
%type <u_pkt_count> pkt_count

%token TOK_EOF 0 "end of file"
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

%token TOK_IPTS_LIMIT

%token TOK_IPTS_LOG_PREFIX

%token TOK_IPTS_UID_OWNER

%token TOK_IPTS_REJECT_WITH

%token TOK_IPTS_CLAMP_MSS_TO_PMTU

%token TOK_IPTS_HELPER

%token <u_str> TOK_IDENTIFIER
%token <u_str> TOK_OPTION

%token TOK_LSQUARE
%token TOK_RSQUARE
%token TOK_COLON
%token TOK_BANG
%token TOK_QUOTE
%token TOK_COMMIT

%start ast
%%

ast: table_list
{
	$$ = (struct ast_s *) malloc(sizeof(struct ast_s));
	driver.result = $$;
	$$->list = $1;
}

table_list: /* empty */
{
  $$ = NULL;
}
| table_list table
{
  $$ = (struct table_list_s *) malloc(sizeof(struct table_list_s));
  $$->list = $1;
  $$->table = $2;
}

table: TOK_IPTS_TABLE TOK_IDENTIFIER rule_list TOK_COMMIT
{
  $$ = (struct table_s *) malloc(sizeof(struct table_s));
  $$->name = $2;
  $$->rule_list = $3;
}

rule_list: /* empty */
{
    $$ = NULL;
}
| rule_list rule
{
    $$ = (struct rule_list_s *) malloc(sizeof(struct rule_list_s));
    $$->list = $1;
    $$->rule = $2;
}

rule: TOK_IPTS_CHAIN TOK_IDENTIFIER TOK_IDENTIFIER pkt_count
{
    $$ = (struct rule_s *) malloc(sizeof(struct rule_s));
    $$->chain = $2;
    $$->policy = $3;
    $$->pkt_count = $4;
    $$->option_list = NULL;
}
| opt_pkt_count chain_command TOK_IDENTIFIER rule_specification
{
    $$ = (struct rule_s *) malloc(sizeof(struct rule_s));
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
    $$ = (struct option_list_s *) malloc(sizeof(struct option_list_s));
    $$->option_list = $1;
    $$->not_option = $2;
}

not_option: TOK_BANG option
{
    $$ = (struct not_option_s *) malloc(sizeof(struct not_option_s));
    $$->neg = 1;
    $$->option = $2;
}
| option
{
    $$ = (struct not_option_s *) malloc(sizeof(struct not_option_s));
    $$->neg = 0;
    $$->option = $1;
}

option: in_interface_option
{
  $$ = (struct option_s *) malloc(sizeof(struct option_s));
  $$->in_interface_option = $1;
}
| jump_option
{
  $$ = (struct option_s *) malloc(sizeof(struct option_s));
  $$->jump_option = $1;
}
| destination_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->destination_option = $1;
}
| protocol_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->protocol_option = $1;
}
| match_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->match_option = $1;
}
| dport_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->dport_option = $1;
}
| to_ports_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->to_ports_option = $1;
}
| source_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->source_option = $1;
}
| out_interface_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->out_interface_option = $1;
}
| to_source_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->to_source_option = $1;
}
| state_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->state_option = $1;
}
| limit_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->limit_option = $1;
}
| log_prefix_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->log_prefix_option = $1;
}
| sport_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->sport_option = $1;
}
| uid_owner_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->uid_owner_option = $1;
}
| tcp_flags_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->tcp_flags_option = $1;
}
| reject_with_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->reject_with_option = $1;
}
| icmp_type_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->icmp_type_option = $1;
}
| fragment_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->fragment_option = $1;
}
| clamp_mss_to_pmtu_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->clamp_mss_to_pmtu_option = $1;
}
| helper_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->helper_option = $1;
}
| syn_option
{
    $$ = (struct option_s *) malloc(sizeof(struct option_s));
    $$->syn_option = $1;
}

syn_option: TOK_IPTS_SYN
{
    $$ = (struct syn_option_s *) malloc(sizeof(struct syn_option_s));
    $$->i = 1;
}

helper_option: TOK_IPTS_HELPER identifier
{
    $$ = (struct helper_option_s *) malloc(sizeof(struct helper_option_s));
    $$->identifier = $2;
}

clamp_mss_to_pmtu_option: TOK_IPTS_CLAMP_MSS_TO_PMTU
{
    $$ = (struct clamp_mss_to_pmtu_option_s *) malloc(sizeof(struct clamp_mss_to_pmtu_option_s));
    $$->i = 1;
}

fragment_option: TOK_IPTS_FRAGMENT
{
    $$ = (struct fragment_option_s *) malloc(sizeof(struct fragment_option_s));
    $$->i = 1;
}

icmp_type_option: TOK_IPTS_ICMP_TYPE identifier
{
    $$ = (struct icmp_type_option_s *) malloc(sizeof(struct icmp_type_option_s));
    $$->identifier = $2;
}

reject_with_option: TOK_IPTS_REJECT_WITH identifier
{
    $$ = (struct reject_with_option_s *) malloc(sizeof(struct reject_with_option_s));
    $$->identifier = $2;
}

tcp_flags_option: TOK_IPTS_TCP_FLAGS identifier identifier
{
    $$ = (struct tcp_flags_option_s *) malloc(sizeof(struct tcp_flags_option_s));
    $$->flags = $2;
    $$->mask = $3;
}

uid_owner_option: TOK_IPTS_UID_OWNER identifier
{
    $$ = (struct uid_owner_option_s *) malloc(sizeof(struct uid_owner_option_s));
    $$->identifier = $2;
}

sport_option: TOK_IPTS_SPORT not_range
{
    $$ = (struct sport_option_s *) malloc(sizeof(struct sport_option_s));
    $$->not_range = $2;
    $$->not_identifier = NULL;
}
| TOK_IPTS_SPORT not_identifier
{
    $$ = (struct sport_option_s *) malloc(sizeof(struct sport_option_s));
    $$->not_range = NULL;
    $$->not_identifier = $2;
}

log_prefix_option: TOK_IPTS_LOG_PREFIX identifier
{
    $$ = (struct log_prefix_option_s *) malloc(sizeof(struct log_prefix_option_s));
    $$->identifier = $2;
}

limit_option: TOK_IPTS_LIMIT identifier
{
    $$ = (struct limit_option_s *) malloc(sizeof(struct limit_option_s));
    $$->identifier = $2;
}

state_option: TOK_IPTS_STATE identifier
{
    $$ = (struct state_option_s *) malloc(sizeof(struct state_option_s));
    $$->identifier = $2;
}

to_source_option: TOK_IPTS_TO_SOURCE identifier
{
    $$ = (struct to_source_option_s *) malloc(sizeof(struct to_source_option_s));
    $$->identifier = $2;
};

out_interface_option: TOK_IPTS_OUT_INTERFACE not_identifier
{
    $$ = (struct out_interface_option_s *) malloc(sizeof(struct out_interface_option_s));
    $$->not_identifier = $2;
}

source_option: TOK_IPTS_SOURCE not_identifier
{
    $$ = (struct source_option_s *) malloc(sizeof(struct source_option_s));
    $$->not_identifier = $2;
}

to_ports_option: TOK_IPTS_TO_PORTS identifier
{
    $$ = (struct to_ports_option_s *) malloc(sizeof(struct to_ports_option_s));
    $$->identifier = $2;
}

dport_option: TOK_IPTS_DPORT not_range
{
    $$ = (struct dport_option_s *) malloc(sizeof(struct dport_option_s));
    $$->not_range = $2;
    $$->not_identifier = NULL;
}
| TOK_IPTS_DPORT not_identifier
{
    $$ = (struct dport_option_s *) malloc(sizeof(struct dport_option_s));
    $$->not_range = NULL;
    $$->not_identifier = $2;
}

match_option: TOK_IPTS_MATCH identifier
{
    $$ = (struct match_option_s *) malloc(sizeof(struct match_option_s));
    $$->identifier = $2;
}

protocol_option: TOK_IPTS_PROTOCOL identifier
{
    $$ = (struct protocol_option_s *) malloc(sizeof(struct protocol_option_s));
    $$->identifier = $2;
}

destination_option: TOK_IPTS_DESTINATION not_identifier
{
    $$ = (struct destination_option_s *) malloc(sizeof(struct destination_option_s));
    $$->not_identifier = $2;
}

jump_option: TOK_IPTS_JUMP identifier
{
  $$ = (struct jump_option_s *) malloc(sizeof(struct jump_option_s));
  $$->identifier = $2;
}

in_interface_option: TOK_IPTS_IN_INTERFACE not_identifier
{
  $$ = (struct in_interface_option_s *) malloc(sizeof(struct in_interface_option_s));
  $$->not_identifier = $2;
}

not_range: TOK_BANG range
{
    $$ = (struct not_range_s *) malloc(sizeof(struct not_range_s));
    $$->neg = 1;
    $$->range = $2;
}
| range
{
    $$ = (struct not_range_s *) malloc(sizeof(struct not_range_s));
    $$->neg = 0;
    $$->range = $1;
}

range: identifier TOK_COLON identifier
{
    $$ = (struct range_s *) malloc(sizeof(struct range_s));
    $$->start = $1;
    $$->end = $3;
}
| identifier TOK_COLON
{
    $$ = (struct range_s *) malloc(sizeof(struct range_s));
    $$->start = $1;
    $$->end = NULL;
}
| TOK_COLON identifier
{
    $$ = (struct range_s *) malloc(sizeof(struct range_s));
    $$->start = NULL;
    $$->end = $2;
}

not_identifier: TOK_BANG identifier
{
    $$ = (struct not_identifier_s *) malloc(sizeof(struct not_identifier_s));
    $$->neg = 1;
    $$->identifier = $2;
}
| identifier
{
    $$ = (struct not_identifier_s *) malloc(sizeof(struct not_identifier_s));
    $$->neg = 0;
    $$->identifier = $1;
}

identifier: TOK_IDENTIFIER
{
    $$ = (struct identifier_s *) malloc(sizeof(struct identifier_s));
    $$->string = $1;
}
| TOK_QUOTE TOK_IDENTIFIER TOK_QUOTE
{
    $$ = (struct identifier_s *) malloc(sizeof(struct identifier_s));
    $$->string = $2;
}

opt_pkt_count: /* empty */
{
    $$ = NULL;
}
| pkt_count
{
    $$ = $1;
}

pkt_count: TOK_LSQUARE TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER TOK_RSQUARE
{
    $$ = (struct pkt_count_s *) malloc(sizeof(struct pkt_count_s));
    $$->in = $2;
    $$->out = $4;
}

%%

void
yy::ipts_parser::error(const yy::ipts_parser::location_type & l,
			     const std::string & m)
{
	driver.error(l, m);
}
