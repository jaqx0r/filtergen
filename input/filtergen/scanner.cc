/* input scanner for filtergen language
 *
 * Copyright (c) 2003-2007 Jamie Wilkinson <jaq@spacepants.org>
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

#include "scanner.h"

FiltergenScanner::FiltergenScanner(std::istream & s):
  source(s),
  currentSpelling("")
{
}

void
FiltergenScanner::accept()
{
  const int c = source.get();
  currentSpelling += c;
}

int
FiltergenScanner::nextToken()
{
  return 0;
}

#if 0
%}

%option c++
 //%option noyywrap nounput
 //%option batch debug
%option prefix="Filtergen"
%option bison-bridge
%option bison-locations

%{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "driver.h"
#include "parser.hh"

void include_file(const char *);
%}

%x include
%x comment

string  \"[^\n]+\"
space   [ \t]+
id      [[:alnum:]_][[:alnum:]_+-]*

%{
#define YY_USER_ACTION yylloc->columns(yyleng);
%}

%%
%{
    yylloc->step();
%}

"/*"	BEGIN(comment);

<comment>{
[^*\n]*		/* eat anything that's not a '*' */
"*"+[^*/\n]*	/* eat up '*'s not followes by '/'s */
[\n]            yylloc->lines(yyleng); yylloc->step();
"*"+"/"		BEGIN(INITIAL);
<<EOF>>		{
    driver.error(*yylloc, "warning: comment reached end of file");
			yyterminate();
		}
}

#[^\n]*      /* strip shell style comments */

{string}     {
		/* we do not store the " characters in the string, so lop
		 * them off.  We can "safely" assume that the first and last
		 * characters in this regex are ", otherwise there's a bug
		 * in flex...  The result is somethign that is syntactically
		 * identical to an identifier for our purposes. */
                yylval->u_str = strndup(yytext+1, yyleng-2);
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
    yylval->u_str = strndup(yytext, yyleng);
    return TOK_IDENTIFIER;
}

"/"          return TOK_SLASH;

.            return TOK_ERR;

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
    char * name;

    name = strdup(yytext);
    include_file(name);
    free(name);

    BEGIN(INITIAL);
}

<<EOF>>                {
    driver.error(*yylloc, "eof! popping state");
    //yypop_buffer_state();

    //if (!YY_CURRENT_BUFFER) {
    //	yyterminate();
    //}
}

%%

/* include a file or directory */
void include_file(const char * name) {
    struct stat st;
    DIR * d;
    struct dirent * r;
    char * fn;

    if (stat(name, &st)) {
	//scan_err("warning: stat failed on \"%s\": %s", name, strerror(errno));
    } else {
	if (S_ISDIR(st.st_mode)) {
	    if ((d = opendir(name)) == NULL) {
		//driver.error(*yylloc, "warning: opendir failed");// on %s: %s", name, strerror(errno));
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
	} else {
	    //scan_err("opening %s as file", name);

	    //yyin = fopen(name, "r");
	    //if ( !yyin ) {
		//scan_err("boned: %s", yytext);
	    //}

	    //yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));

	}
    }
}

// void
// filtergen_driver::scan_begin()
// {
//   //    yy_flex_debug = trace_scanning;

//     if (file)
//       yyin = file;
//     else {
//       if (!(yyin = fopen(filename.c_str(), "r")))
// 	error(std::string("cannot open ") + filename);
//     }
// }

// void
// filtergen_driver::scan_end()
// {
//     fclose(yyin);
// }


#endif
