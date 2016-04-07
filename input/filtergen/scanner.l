%option noyywrap
%option nounput
%option noinput
%option prefix="filtergen_"
%option yylineno
%option debug
%x include
%x comment
%{
/* input scanner for filtergen language
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

#include <libgen.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <glob.h>

#include "input/sourcepos.h"
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/include.h"

int yycolumn = 1;

#define YY_USER_ACTION                                          \
    {                                                           \
        filtergen_lloc.first_line = filtergen_lloc.last_line = yylineno; \
        filtergen_lloc.first_column = yycolumn ;                \
        filtergen_lloc.last_column = yycolumn+yyleng-1;         \
        filtergen_lloc.srcfile = current_srcfile;               \
        current_srcfile->lineno = yylineno;                     \
        yycolumn += yyleng;                                     \
    }

void filtergen_error(struct ast_s *ast, const char *s, ...);
void filtergen_warn(const char * s, ...);
%}

string  \"[^\n]+\"
space   [ \t]+
word    [[:alnum:]._+-]+
id      {word}|({word})?:(({word})?:)+({word})?

%%

\n		/* eat newlines */ { yycolumn = 1; }

"/*"	BEGIN(comment);

<comment>{
[^*\n]*		/* eat anything that's not a '*' */
"*"+[^*/\n]*	/* eat up '*'s not followed by '/'s */
\n		/* eat newlines */ { yycolumn = 1; }
"*"+"/"		BEGIN(INITIAL);
<<EOF>>		{
  filtergen_warn("comment reached end of file");
  yyterminate();
            }
} /* end comment */

#[^\n]*      /* strip shell style comments */

{string}     {
  /* we do not store the " characters in the string, so lop
   * them off.  We can "safely" assume that the first and last
   * characters in this regex are ", otherwise there's a bug
   * in flex...  The result is somethign that is syntactically
   * identical to an identifier for our purposes. */
  filtergen_lval.u_str = strndup(filtergen_text + 1, filtergen_leng - 2);
  return TOK_IDENTIFIER;
}

{space}      /* ignore */

include      BEGIN(include);

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
oneway       return TOK_ONEWAY;
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
"["          return TOK_LSQUARE;
"]"          return TOK_RSQUARE;
";"          return TOK_SEMICOLON;
":"          return TOK_COLON;
"!"          return TOK_BANG;
"*"          return TOK_STAR;

{id}(\.{id})*	{
    filtergen_lval.u_str = strndup(filtergen_text, filtergen_leng);
    return TOK_IDENTIFIER;
}

"/"          return TOK_SLASH;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
    char * name;

    name = strdup(yytext);
    include_file(name);
    free(name);
    
    BEGIN(INITIAL);
}

<<EOF>>                {
    sourcefile_pop();
    yypop_buffer_state();
    if (!YY_CURRENT_BUFFER) {
        yyterminate();
    } else {
        yyin = current_srcfile->f;
        yylineno = current_srcfile->lineno;
        yycolumn = current_srcfile->column;
    }
}

.  { filtergen_error(NULL, "unexpected input: %s", yytext);
        return TOK_UNEXPECTED; }

%%
