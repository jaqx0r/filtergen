/* parser for filtergen
 *
 * Copyright (c) 2003 Jamie Wilkinson <jaq@spacepants.org>
 */

%{
#include <stdio.h>
#include "filter.h"

void yyerror(char * s);
extern int yylex(void);
%}
%debug
%union {
	char * ustr;
	long int uint;
}
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
%token TOK_STRINGLITERAL
%token TOK_TEXT
%token TOK_IDENTIFIER
%token TOK_NUMBER
%token TOK_DOT
%token TOK_SLASH
%token TOK_ERR
%token TOK_BANG
%token TOK_COLON

%%
rule_list: /* empty */
	| rule_list rule
	;

rule:	  TOK_SEMICOLON
	| crap TOK_SEMICOLON
	| list TOK_SEMICOLON
	| crap list TOK_SEMICOLON
	| list crap TOK_SEMICOLON
	| crap list crap TOK_SEMICOLON
	;

list:	  TOK_LCURLY rule TOK_RCURLY
	;

crap:	  /* empty */
	| crap TOK_ACCEPT
	| crap TOK_DEST
	| crap TOK_DPORT
	| crap TOK_DROP
	| crap TOK_FORWARD
	| crap TOK_ICMPTYPE
	| crap TOK_INPUT
	| crap TOK_LOCAL
	| crap TOK_LOG
	| crap TOK_MASQ
	| crap TOK_OUTPUT
	| crap TOK_PROTO
	| crap TOK_PROXY
	| crap TOK_REDIRECT
	| crap TOK_REJECT
	| crap TOK_SOURCE
	| crap TOK_SPORT
	| crap TOK_STRINGLITERAL
	| crap TOK_TEXT
	| crap TOK_IDENTIFIER
	| crap TOK_NUMBER
	| crap TOK_DOT
	| crap TOK_SLASH
	| crap TOK_BANG
	| crap TOK_COLON
	;
	

/*
accept;
ACCEPT SEMICOLON

input eth0 {
	proto tcp dport www accept;
	proto udp dport bootps reject;
	proto tcp dport 57 drop;
	proto tcp sport http:https accept;
	proto tcp source foo dest bar dport 40000:40050 accept;
	reject;
};
INPUT name
 LCURLY
  PROTO name DPORT name ACCEPT SEMICOLON
  PROTO name DPORT name REJECT SEMICOLON
  PROTO name DPORT name DROP SEMICOLON
  PROTO name SPORT name COLON name ACCEPT SEMICOLON
  PROTO name SOURCE name DEST name DPORT name COLON name ACCEPT SEMICOLON
  REJECT SEMICOLON
 RCURLY
SEMICOLON

input eth0 source foo dest bar drop;
INPUT name SOURCE name DEST name DROP SEMICOLON

input eth0 { source foo dest bar } drop;
INPUT name LCURLY
 SOURCE name DEST name
RCURLY DROP SEMICOLON

input eth0 ! source foo dest bar drop;
INPUT name NOT SOURCE name DEST name DROP SEMICOLON

input eth0 ! source foo ! dest bar drop;
INPUT name NOT SOURCE name NOT DEST name DROP SEMICOLON

input eth0 source foo ! dest bar drop;
INPUT name SOURCE name NOT DEST name DROP SEMICOLON

input eth0 ! { source foo dest bar } drop;
INPUT name NOT LCURLY
 SOURCE name DEST name
RCURLY DROP SEMICOLON

input eth0 ! { source foo ! dest bar } drop;
INPUT name NOT LCURLY
 SOURCE foo NOT DEST bar
RCURLY DROP SEMICOLON

input eth0 log drop;
INPUT name LOG DROP SEMICOLON

output eth1 proto tcp log accept;
OUTPUT name PROTO name LOG ACCEPT SEMICOLON

input ppp2 proto tcp log text foo reject;
INPUT name PROTO name LOG TEXT name REJECT SEMICOLON

output ppp4 proto tcp log text "foo bar baz" drop;
OUTPUT name PROTO name LOG TEXT STRING DROP SEMICOLON

input eth0 {
	proto tcp {
		dport 80 redirect;
		dport 22 masq;
	};
	reject;
};
INPUT name LCURLY
 PROTO name LCURLY
  DPORT number REDIRECT SEMICOLON
  DPORT number MASQ SEMICOLON
 RCURLY SEMICOLON
 REJECT SEMICOLON
RCURLY SEMICOLON

output eth0 proto icmp icmptype ping accept;
OUTPUT name PROTO name ICMPTYPE name ACCEPT SEMICOLON

input eth0 proto icmp icmptype pong accept;
INPUT name PROTO name ICMPTYPE name ACCEPT SEMICOLON

input eth0 source wwwserv dest dbserv
[	"web_to_dbserv"
	proto tcp dport { 1521 appserv1 appserv2 appserv3 } accept;
	drop;
];
INPUT name SOURCE name DEST name LSQUARE STRING
 PROTO name DPORT LCURLY
  number name name name
 RCURLY ACCEPT SEMICOLON
 DROP SEMICOLON
RSQUARE SEMICOLON

input eth0 source wwwserv dest dbserv
[	"web_to_dbserv"
	proto tcp dport 1521 accept;
	proto tcp dport appserv1 accept;
	proto tcp dport appserv2 accept;
	proto tcp dport appserv3 accept;
	drop;
];
INPUT name SOURCE name DEST name LSQUARE STRING
 PROTO name DPORT number ACCEPT SEMICOLON
 PROTO name DPORT name ACCEPT SEMICOLON
 PROTO name DPORT name ACCEPT SEMICOLON
 PROTO name DPORT name ACCEPT SEMICOLON
 DROP SEMICOLON
RSQUARE SEMICOLON

input eth0 source wwwserv dest dbserv
[ "web_to_dbserv"
	proto tcp sport 137:139 reject;
	accept;
];
INPUT name SOURCE name DEST name LSQUARE STRING
 PROTO name SPORT number COLON number REJECT SEMICOLON
 ACCEPT;
RSQUARE SEMICOLON

input eth0 source { a b } drop;
INPUT name SOURCE LCURLY
 name name
RCURLY DROP SEMICOLON

input eth0 ! { ! source { a b } } drop;
INPUT name NOT LCURLY
 NOT SOURCE LCURLY
  name name
 RCURLY
RCURLY DROP SEMICOLON

output eth0 {
	{ proto tcp; proto udp } dport domain;
} accept;
OUTPUT name LCURLY
 LCURLY
  PROTO name SEMICOLON
  PROTO name
 RCURLY
 DPORT DOMAIN SEMICOLON
RCURLY ACCEPT SEMICOLON

output eth0 {
	{ proto tcp; proto udp } dport domain;
} accept source bar;
OUTPUT name LCURLY
 LCURLY
  PROTO name SEMICOLON
  PROTO name
 RCURLY
 DPORT DOMAIN SEMICOLON
RCURLY ACCEPT SOURCE name SEMICOLON

input eth0 local {
	dest localhost/8 accept;
	dest tim/255.0.255.0 drop;
};
INPUT name LOCAL LCURLY
 DEST name SLASH number ACCEPT SEMICOLON
 DEST name SLASH number DOT number DOT number DOT number DROP SEMICOLON
RCURLY SEMICOLON

input eth0 {
	source foo accept;
	source bar/24 accept;
	source baz/255.255.255.248 accept;
	dest quux/0 accept;
	dest flirble/32 accept;
	source meep/255.0 drop;
};
INPUT name LCURLY
 SOURCE name ACCEPT SEMICOLON
 SOURCE name SLASH number ACCEPT SEMICOLON
 SOURCE name SLASH number DOT number DOT number DOT number ACCEPT SEMICOLON
 DEST name SLASH number ACCEPT SEMICOLON
 DEST name SLASH number ACCEPT SEMICOLON
 SOURCE name SLASH number DOT number DROP SEMICOLON
RCURLY SEMICOLON

*/
%%
char * filename();
long int lineno();

void yyerror(char * s) {
	fprintf(stderr, "%s:%ld: error: %s\n", filename(), lineno(), s);
}
