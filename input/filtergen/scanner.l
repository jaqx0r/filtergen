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
#include "parser.h"

/* include file stack */
#define MAXINCLUDES 16
struct inc_stack_s {
    YY_BUFFER_STATE state;
    char * filename;
    long int lineno;
};
struct inc_stack_s inc_stack[MAXINCLUDES] = { { state: 0, filename: NULL, lineno: 1 } };

int inc_stackptr = 0;

long int lineno();
char * filename();
static void scan_err(const char * fmt, ...);
void include_file(const char *);
%}

string  \"[^\n]+\"
space   [ \t]+
id      [[:alnum:]_][[:alnum:]_+-]*

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

{id}(\.{id})*	{
			yylval.u_str = strndup(yytext, yyleng);
			return TOK_IDENTIFIER;
		}

"/"          return TOK_SLASH;

.            return TOK_ERR;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
                          if (inc_stackptr >= MAXINCLUDES) {
                              scan_err("warning: too many nested includes");
    			      scan_err("warning: skipping include of file %s", yytext);

			  } else {
				char * name;

				name = strdup(yytext);
				include_file(name);
				free(name);
			  }
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
char * filename(void) {
    return inc_stack[inc_stackptr].filename ?: strdup("(standard input)");
}

static void scan_err(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    if (inc_stackptr >= 0)
	fprintf(stderr, "%s:%ld: ", filename(), lineno());
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}


void step_into_include_file(const char * fn) {
	FILE * f;

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
void include_file(const char * name) {
	struct stat st;
	DIR * d;
	struct dirent * r;
	char * fn;

	if (stat(name, &st)) {
		scan_err("warning: stat failed on %s: %s", name, strerror(errno));
	} else {
		if (S_ISDIR(st.st_mode)) {
			if ((d = opendir(name)) == NULL) {
				scan_err("warning: opendir failed on %s: %s", name, strerror(errno));
			} else {
				while ((r = readdir(d)) != NULL) {
					/* FIXME: assumes d_name */
					if (r->d_name[0] == '.')
						continue;
					asprintf(&fn, "%s/%s", name, r->d_name);
					include_file(fn);
					free(fn);
				}
				closedir(d);
			}
		} else
			step_into_include_file(name);
	}
}
