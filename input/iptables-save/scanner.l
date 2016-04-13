%top{
/* input scanner for iptables-save format
 *
 * Copyright (c) 2004 Jamie Wilkinson <jaq@spacepants.org>
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

#include <string.h>

#include "input/iptables-save/ast.h"
#include "input/iptables-save/parser.h"

#define YYSTYPE IPTS_STYPE
#define YYLTYPE IPTS_LTYPE
}
%option 8bit
%option bison-bridge
%option bison-locations
%option noyywrap
%option nounput
%option noinput
%option nostack
%option warn
%option prefix="ipts_"
%option yylineno
%option debug
%option nointeractive

%{
int ipts_column = 1;

#define YY_USER_ACTION                                                         \
  {                                                                            \
    yylloc->first_line = yylloc->last_line = yylineno;                         \
    yylloc->first_column = ipts_column;                                        \
    yylloc->last_column = ipts_column + yyleng - 1;                            \
    ipts_column += yyleng;                                                     \
  }
%}

string	\"[^\n]+\"
space	[ \t]+
id	[[:alnum:]_+-\.\/~]+

%%

#[^\n]* 	/* strip shell-style comments */

^[*]		return TOK_IPTS_TABLE;

^:		return TOK_IPTS_CHAIN;

{space}	/* ignore */

\n		/* ignore */ { ipts_column = 1; }

^COMMIT		return TOK_COMMIT;

{string}	{
		/* we do not store the " characters in the string, so lop
                 * them off.  We can "safely" assume that the first and last
                 * characters in this regex are ", otherwise there's a bug
                 * in flex...  The result is somethign that is syntactically
                 * identical to an identifier for our purposes. */
                yylval->u_str = strndup(yytext + 1, yyleng - 2);
                return TOK_IDENTIFIER;
                }

		/* chain commands */
-A		|
--append	return TOK_IPTS_CHAIN_APPEND;

-p		|
--protocol	return TOK_IPTS_PROTOCOL;

-s		|
--source	return TOK_IPTS_SOURCE;

-d		|
--destination	return TOK_IPTS_DESTINATION;

-j		|
--jump		return TOK_IPTS_JUMP;

-i		|
--in-interface	return TOK_IPTS_IN_INTERFACE;

-o		|
--out-interface	return TOK_IPTS_OUT_INTERFACE;

-f		|
--fragment	return TOK_IPTS_FRAGMENT;

-c		|
--set-counters	return TOK_IPTS_SET_COUNTERS;

-m		|
--match		return TOK_IPTS_MATCH;

		/* match arguments */
--dport		|
--destination-port	return TOK_IPTS_DPORT;
--sport		|
--source-port	return TOK_IPTS_SPORT;

--sports	|
--source-ports	return TOK_IPTS_SOURCE_PORTS;
--dports	|
--destination-ports	return TOK_IPTS_DESTINATION_PORTS;
--ports		return TOK_IPTS_PORTS;

--tcp-flags	return TOK_IPTS_TCP_FLAGS;
--syn		return TOK_IPTS_SYN;
--tcp-option	return TOK_IPTS_TCP_OPTION;
--mss		return TOK_IPTS_MSS;

--state		return TOK_IPTS_STATE;

--icmp-type	return TOK_IPTS_ICMP_TYPE;

--to-ports	return TOK_IPTS_TO_PORTS;
--to-source	return TOK_IPTS_TO_SOURCE;

--limit		return TOK_IPTS_LIMIT;

--log-prefix	return TOK_IPTS_LOG_PREFIX;

--uid-owner	return TOK_IPTS_UID_OWNER;

--reject-with	return TOK_IPTS_REJECT_WITH;

--clamp-mss-to-pmtu	return TOK_IPTS_CLAMP_MSS_TO_PMTU;

--helper	return TOK_IPTS_HELPER;

	/* everything else */
{id}	{
		yylval->u_str = strndup(yytext, yyleng);
		return TOK_IDENTIFIER;
	}

"["	return TOK_LSQUARE;
"]"	return TOK_RSQUARE;
":"	return TOK_COLON; 
"!"	return TOK_BANG;

\"	return TOK_QUOTE;

%%
