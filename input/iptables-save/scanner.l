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

long int ipts_ln = 1;

char * ipts_filename(void);
%}

string	\"[^\n]+\"
space	[ \t]+
id	[[:alnum:]_+-\.\/]+

%%

#[^\n]*[\n] { /* strip shell-style comments and the trailing newline */ 
		ipts_ln++;
	    }

^[*]	return TOK_TABLE;

^:	return TOK_CHAIN;

{space}	/* ignore */

\n	{
		ipts_ln++;
		return TOK_NEWLINE;
	}

^COMMIT	return TOK_COMMIT;

{string}	{
		/* we do not store the " characters in the string, so lop
                 * them off.  We can "safely" assume that the first and last
                 * characters in this regex are ", otherwise there's a bug
                 * in flex...  The result is somethign that is syntactically
                 * identical to an identifier for our purposes. */
                ipts_lval.u_str = strndup(ipts_text + 1, ipts_leng - 2);
                return TOK_IDENTIFIER;
                }

-{id}	{
		/* ignore the first character */
		ipts_lval.u_str = strndup(ipts_text + 1, ipts_leng - 1);
		return TOK_OPTION;
	}

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
