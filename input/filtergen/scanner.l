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

#include "input/filtergen/ast.h"
#include "input/sourcepos.h"
#include "input/filtergen/parser.h"

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

.  { filtergen_error("unexpected input: %s\n", yytext);
        return TOK_UNEXPECTED; }

%%

/* include a file or directory */
void include_file(const char *name) {
  struct stat st;
  struct dirent **namelist;
  char *fn;
  int n;

  if (stat(name, &st)) {
    if (errno == ENOENT &&
        (index(name, '*') != NULL || index(name, '?') != NULL ||
         index(name, '[') != NULL)) {
      /* Globbing fiesta! */
      glob_t glob_buf;
      if (glob(name, 0, NULL, &glob_buf) != 0) {
        filtergen_warn("failed to glob \"%s\": %s", name, strerror(errno));
      } else {
        /* We go through the list of files backwards, because
         * step_into_include_file() creates a stack of all the
         * files processed and then works on them in a LIFO
         * fashion -- which would make all of our rules files
         * go backwards.  Since I can't wrap my head around
         * why that is, exactly, I'm hacking it up with
         * this instead.  Fixination appreciated.
         */
        for (n = glob_buf.gl_pathc - 1; n >= 0; n--) {
          if (stat(glob_buf.gl_pathv[n], &st)) {
            filtergen_warn("stat failed on globbed \"%s\": %s",
                     glob_buf.gl_pathv[n], strerror(errno));
          } else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            sourcefile_push(glob_buf.gl_pathv[n]);
            yyin = current_srcfile->f;
            yylineno = current_srcfile->lineno;
            yycolumn = current_srcfile->column;
            if (!yyin) {
                filtergen_error("failed to open %s", glob_buf.gl_pathv[n]);
            } else {
                yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
            }
          }
        }
      }

      globfree(&glob_buf);
    } else {
      filtergen_warn("stat failed on \"%s\": %s", name, strerror(errno));
    }
  } else {
    if (S_ISDIR(st.st_mode)) {
      char *b = strdup(name);
      char *base = basename(b);

      if (strcmp("/", base) == 0) {
        filtergen_warn("cannot include / path; skipping");
        free(b);
        return;
      }
      free(b);

      if ((n = scandir(name, &namelist, NULL, alphasort)) < 0) {
        filtergen_warn("scandir failed on \"%s\": %s", name, strerror(errno));
      } else {
        while (n--) {
          /* FIXME: assumes d_name */
          if (namelist[n]->d_name[0] == '.') {
            free(namelist[n]);
            continue;
          }
          if (asprintf(&fn, "%s/%s", name, namelist[n]->d_name) < 0) {
            filtergen_error("internal error: asprintf failed constructing pathname "
                     "for included file \"%s\"",
                     namelist[n]->d_name);
            free(fn);
          } else {
            include_file(fn);
            free(fn);
          }
          free(namelist[n]);
        }
        free(namelist);
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
