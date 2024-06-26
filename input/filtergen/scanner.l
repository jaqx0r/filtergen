%top{
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


#include "input/sourcepos.h"
#include "input/sourcefile.h"
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/include.h"

#define YYSTYPE FILTERGEN_STYPE
#define YYLTYPE FILTERGEN_LTYPE
}
%option 8bit
%option bison-bridge
%option bison-locations
%option noyywrap
%option nounput
%option noinput
%option warn
%option prefix="filtergen_"
%option yylineno
%option debug
%option nointeractive
%x include
%x comment

%{
int filtergen_column = 1;

#define YY_USER_ACTION                                                         \
  {                                                                            \
    yylloc->first_line = yylloc->last_line = yylineno;                         \
    yylloc->first_column = filtergen_column;                                   \
    yylloc->last_column = filtergen_column + yyleng - 1;                       \
    yylloc->filename = current_srcfile->pathname;                              \
    current_srcfile->lineno = yylineno;                                        \
    filtergen_column += yyleng;                                                \
  }
%}

string  \"[^\n]+\"
space   [ \t]+
word    [[:alnum:]._+-]+
id      {word}|({word})?:(({word})?:)+({word})?

%%

\n		/* eat newlines */ { filtergen_column = 1; }

"/*"	BEGIN(comment);

<comment>{
[^*\n]*		/* eat anything that's not a '*' */
"*"+[^*/\n]*	/* eat up '*'s not followed by '/'s */
\n		/* eat newlines */ { filtergen_column = 1; }
"*"+"/"		BEGIN(INITIAL);
<<EOF>>		{
  filter_error(make_sourcepos(yylloc), "comment reached end of file");
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
  yylval->u_str = strndup(yytext + 1, yyleng - 2);
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
    yylval->u_str = strndup(yytext, yyleng);
    return TOK_IDENTIFIER;
}

"/"          return TOK_SLASH;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
    char * name;

    name = strdup(yytext);
    include_file(yylloc, name);
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
        filtergen_column = current_srcfile->column;
    }
}

.  { filter_error(make_sourcepos(yylloc), "unexpected input: %s", yytext);
        return TOK_UNEXPECTED; }

%%
