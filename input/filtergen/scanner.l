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

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "ast.h"
#include "../sourcepos.h"
#include "parser.h"

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

void filtergen_error(const char * s, ...);
void filtergen_warn(const char * s, ...);
void include_file(const char *);
%}

string  \"[^\n]+\"
space   [ \t]+
id      [[:alnum:]_][[:alnum:]_+-]*

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

{id}(\.{id})*	{
    filtergen_lval.u_str = strndup(filtergen_text, filtergen_leng);
    return TOK_IDENTIFIER;
}

"/"          return TOK_SLASH;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
    char * name;

    name = strdup(yytext);
    //filtergen_error("including %s", name);
    include_file(name);
    free(name);

    BEGIN(INITIAL);
}

<<EOF>>                {
    //filtergen_error("eof! popping state");
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

.  { filtergen_error("unexpected input: %s\n", yytext);
        return TOK_UNEXPECTED; }

%%

/* include a file or directory */
void include_file(const char * name) {
    struct stat st;
    DIR * d;
    struct dirent * r;
    char * pathname;

    if (stat(name, &st)) {
        filtergen_warn("stat failed on \"%s\": %s", name, strerror(errno));
    } else {
        if (S_ISDIR(st.st_mode)) {
            if ((d = opendir(name)) == NULL) {
                filtergen_warn("opendir failed on %s: %s", name, strerror(errno));
            } else {
                while ((r = readdir(d)) != NULL) {
                    /* FIXME: assumes d_name */
                    if (r->d_name[0] == '.')
                        continue;
                    if (asprintf(&pathname, "%s/%s", name, r->d_name) < 0) {
                        filtergen_error("internal error: asprintf failed constructing pathname for included file %s", r->d_name);
                    } else {
                        include_file(pathname);
                        free(pathname);
                    }
                }
                closedir(d);
            }
        } else {
            sourcefile_push(name);
            yyin = current_srcfile->f;
            yylineno = current_srcfile->lineno;
            yycolumn = current_srcfile->column;
            if (!yyin) {
                filtergen_error("failed to open %s", name);
            } else {
                yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
            }
        }
    }
}
