%option noyywrap
%option nounput
%option prefix="ipts_"
%option yylineno

%{
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
#include "parser.h"

char * ipts_filename(void);
%}

string	\"[^\n]+\"
space	[ \t]+
id	[[:alnum:]_+-\.\/]+

%%

#[^\n]* 	/* strip shell-style comments */

^[*]		return TOK_IPTS_TABLE;

^:		return TOK_IPTS_CHAIN;

{space}	/* ignore */

\n		/* ignore */

^COMMIT		return TOK_COMMIT;

{string}	{
		/* we do not store the " characters in the string, so lop
                 * them off.  We can "safely" assume that the first and last
                 * characters in this regex are ", otherwise there's a bug
                 * in flex...  The result is somethign that is syntactically
                 * identical to an identifier for our purposes. */
                ipts_lval.u_str = strndup(ipts_text + 1, ipts_leng - 2);
                return TOK_IDENTIFIER;
                }

		/* chain commands */
-A		return TOK_IPTS_CHAIN_APPEND;
--append	return TOK_IPTS_CHAIN_APPEND;

-p		return TOK_IPTS_PROTOCOL;
--protocol	return TOK_IPTS_PROTOCOL;

-s		return TOK_IPTS_SOURCE;
--source	return TOK_IPTS_SOURCE;

-d		return TOK_IPTS_DESTINATION;
--destination	return TOK_IPTS_DESTINATION;

-j		return TOK_IPTS_JUMP;
--jump		return TOK_IPTS_JUMP;

-i		return TOK_IPTS_IN_INTERFACE;
--in-interface	return TOK_IPTS_IN_INTERFACE;

-o		return TOK_IPTS_OUT_INTERFACE;
--out-interface	return TOK_IPTS_OUT_INTERFACE;

-f		return TOK_IPTS_FRAGMENT;
--fragment	return TOK_IPTS_FRAGMENT;

-c		return TOK_IPTS_SET_COUNTERS;
--set-counters	return TOK_IPTS_SET_COUNTERS;

-m		return TOK_IPTS_MATCH;
--match		return TOK_IPTS_MATCH;

		/* match arguments */
--dport		return TOK_IPTS_DPORT;
--destination-port	return TOK_IPTS_DPORT;
--sport		return TOK_IPTS_SPORT;
--source-port	return TOK_IPTS_SPORT;

--sports	return TOK_IPTS_SOURCE_PORTS;
--source-ports	return TOK_IPTS_SOURCE_PORTS;
--dports	return TOK_IPTS_DESTINATION_PORTS;
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

	/* everything else */
{id}	{
		ipts_lval.u_str = strndup(ipts_text, ipts_leng);
		return TOK_IDENTIFIER;
	}

"["	return TOK_LSQUARE;
"]"	return TOK_RSQUARE;
":"	return TOK_COLON; 
"!"	return TOK_BANG;

\"	return TOK_QUOTE;

%%

char * ipts_filename(void) {
	return NULL;
}
