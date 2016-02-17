%option noyywrap
%option nounput
%x include
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
#include <glob.h>
#include "parser.h"

/* include file stack */
#define MAXINCLUDES 512
struct inc_stack_s {
    YY_BUFFER_STATE state;
    char * filename;
    long int lineno;
};
struct inc_stack_s inc_stack[MAXINCLUDES] = { { .state = 0, .filename = NULL, .lineno =  1 } };

int inc_stackptr = 0;

long int lineno();
char * filename();
static void scan_err(const char * fmt, ...) __attribute__((format(printf, 1, 2)));
void include_file(const char *);
%}

string  \"[^\n]+\"
space   [ \t]+
word    [[:alnum:]._+-]+
id      {word}|({word})?:(({word})?:)+({word})?

%%

"/*"         { /* strip c-style comments */
                int c;
                do {
                    while ((c = input()) != '*' && c != EOF && c != '\n')
			;
                    while (c == '*')
                        c = input();
                    if (c == EOF)
                        scan_err("comment reached end of file");
                    if (c == '\n')
                        inc_stack[inc_stackptr].lineno++;
                } while (c != '/' && c != EOF);
             }

#[^\n]*      /* strip shell style comments */

{string}     {
		/* we do not store the " characters in the string, so lop
		 * them off.  We can "safely" assume that the first and last
		 * characters in this regex are ", otherwise there's a bug
		 * in flex...  The result is somethign that is syntactically
		 * identical to an identifier for our purposes. */
		yylval.u_str = strndup(yytext + 1, yyleng - 2);
		return TOK_IDENTIFIER;
             }

{space}      /* ignore */

\n           inc_stack[inc_stackptr].lineno++;

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

{id}		{
			yylval.u_str = strndup(yytext, yyleng);
			return TOK_IDENTIFIER;
		}

"/"          return TOK_SLASH;

.            return TOK_ERR;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
			  char * name = strdup(yytext);
			  include_file(name);
			  free(name);
                          BEGIN(INITIAL);
                       }

<<EOF>>                {
                           if (!inc_stackptr) {
                              yyterminate();
			   } else {
                             if (inc_stack[inc_stackptr].filename) {
                               free(inc_stack[inc_stackptr].filename);
                               inc_stack[inc_stackptr].filename = NULL;
                             }
                             inc_stackptr--;
                             yy_delete_buffer(YY_CURRENT_BUFFER);
                             yy_switch_to_buffer(inc_stack[inc_stackptr].state);
                           }
			}
%%

long int lineno(void) {
  return inc_stack[inc_stackptr].lineno;
}

/* FIXME: make this return an immutable string */
char *filename(void) {
  return inc_stack[inc_stackptr].filename ? inc_stack[inc_stackptr].filename
                                          : strdup("(standard input)");
}

static void scan_err(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  if (inc_stackptr >= 0)
    fprintf(stderr, "%s:%ld: ", filename(), lineno());
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
}

void step_into_include_file(const char *fn) {
  FILE *f;

  if (inc_stackptr >= MAXINCLUDES - 1) {
    scan_err("warning: too many nested includes");
    scan_err("warning: skipping include of file \"%s\"", fn);
    return;
  }

  if (!(f = fopen(fn, "r"))) {
    scan_err("warning: can't open file \"%s\"", fn);
  } else {
    inc_stack[inc_stackptr++].state = YY_CURRENT_BUFFER;
    inc_stack[inc_stackptr].lineno = 1;
    inc_stack[inc_stackptr].filename = strdup(fn);
    yyin = f;
    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
  }
}

/* include a file or directory */
void include_file(const char *name) {
  struct stat st;
  struct dirent **namelist;
  char *fn;
  int n;

  if (strlen(name) == 1 && name[0] == '/' && name[1] == '\0') {
    scan_err("warning: cannot include / path; skipping");
    return;
  }

  if (stat(name, &st)) {
    if (errno == ENOENT &&
        (index(name, '*') != NULL || index(name, '?') != NULL ||
         index(name, '[') != NULL)) {
      /* Globbing fiesta! */
      glob_t glob_buf;
      if (glob(name, 0, NULL, &glob_buf) != 0) {
        scan_err("warning: failed to glob %s: %s", name, strerror(errno));
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
            scan_err("warning: stat failed on globbed %s: %s",
                     glob_buf.gl_pathv[n], strerror(errno));
          } else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            step_into_include_file(glob_buf.gl_pathv[n]);
          }
        }
      }

      globfree(&glob_buf);
    } else {
      scan_err("warning: stat failed on %s: %s", name, strerror(errno));
    }
  } else {
    if (S_ISDIR(st.st_mode)) {
      if ((n = scandir(name, &namelist, NULL, alphasort)) < 0) {
        scan_err("warning: scandir failed on %s: %s", name, strerror(errno));
      } else {
        while (n--) {
          /* FIXME: assumes d_name */
          if (namelist[n]->d_name[0] == '.') {
	    free(namelist[n]);
            continue;
          }
          if (asprintf(&fn, "%s/%s", name, namelist[n]->d_name) < 0) {
            scan_err("internal error: asprintf failed constructing pathname "
                     "for included file %s",
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
      step_into_include_file(name);
    }
  }
}
