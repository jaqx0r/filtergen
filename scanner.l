%option noyywrap
%option nounput
%x include
%{
/* input scanner for filtergen
 *
 * Copyright (c) 2003 Jamie Wilkinson <jaq@spacepants.org>
 */

#include <string.h>
#include <stdarg.h>
#include "parser.h"

/* include file stack */
#define MAXINCLUDES 16
struct inc_stack_s {
    YY_BUFFER_STATE state;
    char * filename;
    long int lineno;
};
struct inc_stack_s inc_stack[MAXINCLUDES] = { { filename: NULL, lineno: 1 } };

int inc_stackptr = 0;

long int lineno();
char * filename();
static void scan_err(const char * fmt, ...);
%}

string  \"[^\n]+\"
space   [ \t]+
id      [[:alpha:]_][[:alnum:]_-]*
number  [[:digit:]]+

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
{number}     	{
			yylval.u_int = atoi(yytext);
			return TOK_NUMBER;
		}
"."          return TOK_DOT;
"/"          return TOK_SLASH;

.            return TOK_ERR;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n]+    { /* include file name */
                          if (inc_stackptr >= MAXINCLUDES) {
                              scan_err("warning: too many nested includes");
    			      scan_err("warning: skipping include of file %s", yytext);

			  } else {
                              if (yytext) {
                                  FILE * f;
                                  if (!(f = fopen(yytext, "r"))) {
                                      scan_err("error: can't open file \"%s\"", yytext);
                                  } else {
                                      inc_stack[inc_stackptr++].state = YY_CURRENT_BUFFER;
                                      inc_stack[inc_stackptr].lineno = 1;
                                      inc_stack[inc_stackptr].filename = strdup(yytext);
                                      yyin = f;
                                      yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
                                  }
                              } else {
                                  scan_err("error: file is null");
                              }    
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
    return inc_stack[inc_stackptr].filename ?: "(standard input)";
}

static void scan_err(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    if (inc_stackptr >= 0)
	fprintf(stderr, "%s:%ld: ", filename(), lineno());
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}
