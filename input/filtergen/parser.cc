/* parser for the filtergen language
 *
 * Copyright (c) 2003-2007 Jamie Wilkinson <jaq@spacepants.org>
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

#include "parser.h"

FiltergenParser::FiltergenParser(Scanner & _scanner):
  scanner(_scanner)
{
  accept();
}

void
FiltergenParser::accept()
{
  currentToken = scanner.getToken();
}

bool
FiltergenParser::match(const Token & expected)
{
  if (*currentToken == expected) {
    accept();
    return true;
  } else
    return false;
}

void
FiltergenParser::parse()
{
  //parseRuleList();
}

// void
// FiltergenParser::parseRuleList()
// {
//   parseRule();
//   parseRuleList();
// }


#if 0

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
	;

rule:	  specifier_list TOK_SEMICOLON
	{
	    $$ = (struct rule_s *) malloc(sizeof(struct rule_s));
	    $$->list = $1;
	}
	;
#endif //0

void
FiltergenParser::parseRule()
{
  //parseSpecifierList();
  match(Token::SEMI);
}

#if 0

specifier_list: /* empty */
	{
	    $$ = NULL;
	}
	| specifier_list negated_specifier
	{
	    $$ = (struct specifier_list_s *) malloc(sizeof(struct specifier_list_s));
	    $$->list = $1;
	    $$->spec = $2;
	}
	;

negated_specifier: specifier
	{
	    $$ = (struct negated_specifier_s *) malloc(sizeof(struct negated_specifier_s));
	    $$->negated = 0;
	    $$->spec = $1;
	}
	| TOK_BANG specifier
	{
	    $$ = (struct negated_specifier_s *) malloc(sizeof(struct negated_specifier_s));
	    $$->negated = 1;
	    $$->spec = $2;
	}
	;

specifier: compound_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->compound = $1;
	}
	| direction_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->direction = $1;
	}
	| target_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->target = $1;
	}
	| host_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->host = $1;
	}
	| port_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->port = $1;
	}
	| protocol_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->protocol = $1;
	}
	| icmptype_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->icmptype = $1;
	}
	| option_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->option = $1;
	}
	| chaingroup_specifier
	{
	    $$ = (struct specifier_s *) malloc(sizeof(struct specifier_s));
	    memset($$, 0, sizeof(struct specifier_s));
	    $$->chaingroup = $1;
	}
	;

direction_specifier: TOK_INPUT direction_argument_list
	{
	    $$ = (struct direction_specifier_s *) malloc(sizeof(struct direction_specifier_s));
	    $$->type = TOK_INPUT;
	    $$->list = $2;
	}
	| TOK_OUTPUT direction_argument_list
	{
	    $$ = (struct direction_specifier_s *) malloc(sizeof(struct direction_specifier_s));
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
	    $$ = (struct direction_argument_list_s *) malloc(sizeof(struct direction_argument_list_s));
	    $$->list = $1;
	    $$->arg = $2;
	}
	;

direction_argument: TOK_IDENTIFIER
	{
	    $$ = (struct direction_argument_s *) malloc(sizeof(struct direction_argument_s));
	    $$->direction = $1;
	}
	;

target_specifier: TOK_ACCEPT
	{
	    $$ = (struct target_specifier_s *) malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_ACCEPT;
	}
	| TOK_REJECT
	{
	    $$ = (struct target_specifier_s *) malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_REJECT;
	}
	| TOK_DROP
	{
	    $$ = (struct target_specifier_s *) malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_DROP;
	}
	| TOK_MASQ
	{
	    $$ = (struct target_specifier_s *) malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_MASQ;
	}
	| TOK_PROXY
	{
	    $$ = (struct target_specifier_s *) malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_PROXY;
	}
	| TOK_REDIRECT
	{
	    $$ = (struct target_specifier_s *) malloc(sizeof(struct target_specifier_s));
	    $$->type = TOK_REDIRECT;
	}
	;

host_specifier: TOK_SOURCE host_argument_list
	{
	    $$ = (struct host_specifier_s *) malloc(sizeof(struct host_specifier_s));
	    $$->type = TOK_SOURCE;
	    $$->list = $2;
	}
	| TOK_DEST host_argument_list
	{
	    $$ = (struct host_specifier_s *) malloc(sizeof(struct host_specifier_s));
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
	    $$ = (struct host_argument_list_s *) malloc(sizeof(struct host_argument_list_s));
	    $$->list = $1;
	    $$->arg = $2;
	}
	;

host_argument: TOK_IDENTIFIER TOK_SLASH TOK_IDENTIFIER
	{
	    $$ = (struct host_argument_s *) malloc(sizeof(struct host_argument_s));
	    $$->host = $1;
	    $$->mask = $3;
	}
        | TOK_IDENTIFIER
        {
	    $$ = (struct host_argument_s *) malloc(sizeof(struct host_argument_s));
	    $$->host = $1;
	    $$->mask = 0;
	}
	;

port_specifier: TOK_SPORT port_argument_list
	{
	    $$ = (struct port_specifier_s *) malloc(sizeof(struct port_specifier_s));
	    $$->type = TOK_SPORT;
	    $$->list = $2;
	}
	| TOK_DPORT port_argument_list
	{
	    $$ = (struct port_specifier_s *) malloc(sizeof(struct port_specifier_s));
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
	    $$ = (struct port_argument_list_s *) malloc(sizeof(struct port_argument_list_s));
	    $$->list = NULL;
	    $$->arg = $1;
	}
	| port_argument_list_ port_argument
	{
	    $$ = (struct port_argument_list_s *) malloc(sizeof(struct port_argument_list_s));
	    $$->list = $1;
	    $$->arg = $2;
	}
	;

port_argument: TOK_IDENTIFIER TOK_COLON TOK_IDENTIFIER
	{
	    $$ = (struct port_argument_s *) malloc(sizeof(struct port_argument_s));
	    $$->port_min = $1;
	    $$->port_max = $3;
	}
	| TOK_IDENTIFIER
	{
	    $$ = (struct port_argument_s *) malloc(sizeof(struct port_argument_s));
	    $$->port_min = $1;
	    $$->port_max = NULL;
	}
	;

protocol_specifier: TOK_PROTO protocol_argument_list
	{
	    $$ = (struct protocol_specifier_s *) malloc(sizeof(struct protocol_specifier_s));
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
	    $$ = (struct protocol_argument_list_s *) malloc(sizeof(struct protocol_argument_list_s));
	    $$->list = $1;
	    $$->arg = $2;
	}
	;

protocol_argument: TOK_IDENTIFIER
	{
	    $$ = (struct protocol_argument_s *) malloc(sizeof(struct protocol_argument_s));
	    $$->proto = strdup($1);
	}
	;

icmptype_specifier: TOK_ICMPTYPE icmptype_argument_list
	{
	    $$ = (struct icmptype_specifier_s *) malloc(sizeof(struct icmptype_specifier_s));
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
	    $$ = (struct icmptype_argument_list_s *) malloc(sizeof(struct icmptype_argument_list_s));
	    $$->list = $1;
	    $$->arg = $2;
	}
	;

icmptype_argument: TOK_IDENTIFIER
	{
	    $$ = (struct icmptype_argument_s *) malloc(sizeof(struct icmptype_argument_s));
	    $$->icmptype = $1;
	}
	;

option_specifier: TOK_LOCAL
	{
	    $$ = (struct option_specifier_s *) malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_LOCAL;
	    $$->logmsg = 0;
	}
	| TOK_FORWARD
	{
	    $$ = (struct option_specifier_s *) malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_FORWARD;
	    $$->logmsg = 0;
	}
	| TOK_ONEWAY
	{
	    $$ = (struct option_specifier_s *) malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_ONEWAY;
	    $$->logmsg = 0;
	}
        | TOK_LOG TOK_TEXT TOK_IDENTIFIER
        {
	    $$ = (struct option_specifier_s *) malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_LOG;
	    $$->logmsg = $3;
	}
	| TOK_LOG
	{
	    $$ = (struct option_specifier_s *) malloc(sizeof(struct option_specifier_s));
	    $$->type = TOK_LOG;
	    $$->logmsg = 0;
	}
	;

compound_specifier: TOK_LCURLY subrule_list TOK_RCURLY
	{
	    $$ = (struct compound_specifier_s *) malloc(sizeof(struct compound_specifier_s));
	    $$->list = $2;
	}
	;

subrule_list: specifier_list
	{
	    $$ = (struct subrule_list_s *) malloc(sizeof(struct subrule_list_s));
	    $$->subrule_list = NULL;
	    $$->specifier_list = $1;
	}
	| subrule_list TOK_SEMICOLON specifier_list
	{
	    $$ = (struct subrule_list_s *) malloc(sizeof(struct subrule_list_s));
	    $$->subrule_list = $1;
	    $$->specifier_list = $3;
	}
	;

chaingroup_specifier: TOK_LSQUARE TOK_IDENTIFIER subrule_list TOK_RSQUARE
	{
	    $$ = (struct chaingroup_specifier_s *) malloc(sizeof(struct chaingroup_specifier_s));
	    $$->name = $2;
	    $$->list = $3;
	}

#endif // 0

void
FiltergenParser::parseChaingroupSpecifier()
{
    match(Token::LSQUARE);
    match(Token::RSQUARE);
}

#if 0
	| TOK_LSQUARE subrule_list TOK_RSQUARE
	{
	    $$ = (struct chaingroup_specifier_s *) malloc(sizeof(struct chaingroup_specifier_s));
	    $$->name = NULL;
	    $$->list = $2;
	}
	;

%%

void
yy::filtergen_parser::error(const yy::filtergen_parser::location_type & l,
			    const std::string & m)
{
    driver.error(l, m);
}
#endif
