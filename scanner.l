%option noyywrap
%option nounput
%{
/* input scanner for filtergen
 *
 * Copyright (c) 2003 Jamie Wilkinson <jaq@spacepants.org>
 */

#include <string.h>
#include "tokens.h"

long int lineno = 1;
%}

string  \"[^\n]+\"
space   [ \t]+
id      [[:alpha:]_][[:alnum:]_]*
number  [[:digit:]]+

%%

"/*"         { /* strip c-style comments */
                int c;
                do {
                    while ((c = input()) != '*' && c != EOF && c != '\n')
			;
                    while (c == '*')
                        c = input();
                    /*
                    if (c == EOF)
                        yyerror("comment reached end of file");
		    */
                    if (c == '\n')
                        lineno++;
                } while (c != '/' && c != EOF);
             }

"#"          { /* strip shell style comments */
                 int c;
                 while ((c = input()) != '\n' && c != EOF)
		    ;
             }

{string}     {
                 /* spelling = strdup(yytext); */
                 return TOK_STRINGLITERAL;
             }

{space}      /* ignore */

\n           lineno++;

include      return TOK_INCLUDE;

accept       return TOK_ACCEPT;
dest         return TOK_DEST;
dport        return TOK_DPORT;
drop         return TOK_DROP;
forward      return TOK_FORWARD;
icmptype     return TOK_ICMPTYPE;
input        return TOK_INPUT;
local        return TOK_LOCAL;
log          return TOK_LOG;
masq         return TOK_MASQ;
output       return TOK_OUTPUT;
proto        return TOK_PROTO;
proxy        return TOK_PROXY;
redirect     return TOK_REDIRECT;
reject       return TOK_REJECT;
source       return TOK_SOURCE;
sport        return TOK_SPORT;
text         return TOK_TEXT;

"{"          return TOK_LCURLY;
"}"          return TOK_RCURLY;
";"          return TOK_SEMICOLON;
"["          return TOK_LSQUARE;
"]"          return TOK_RSQUARE;

{id}         return TOK_IDENTIFIER;
{number}     return TOK_NUMBER;
"."          return TOK_DOT;
"/"          return TOK_SLASH;

.            return TOK_ERR;
%%
