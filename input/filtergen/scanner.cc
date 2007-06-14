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

#include <cctype>

#include "scanner.h"

FiltergenScanner::FiltergenScanner(std::istream & s):
  source(s),
  lexeme("")
{
  keywords["accept"] = &Token::ACCEPT;
  keywords["dest"] = &Token::DEST;
  keywords["dport"] = &Token::DPORT;
  keywords["drop"] = &Token::DROP;
  keywords["forward"] = &Token::FORWARD;
  keywords["icmptype"] = &Token::ICMPTYPE;
  keywords["input"] = &Token::INPUT;
  keywords["local"] = &Token::LOCAL;
  keywords["log"] = &Token::LOG;
  keywords["masq"] = &Token::MASQ;
  keywords["oneway"] = &Token::ONEWAY;
  keywords["output"] = &Token::OUTPUT;
  keywords["proto"] = &Token::PROTO;
  keywords["proxy"] = &Token::PROXY;
  keywords["redirect"] = &Token::REDIRECT;
  keywords["reject"] = &Token::REJECT;
  keywords["source"] = &Token::SOURCE;
  keywords["sport"] = &Token::SPORT;
  keywords["text"] = &Token::TEXT;
}

void
FiltergenScanner::accept(const bool append)
{
  const int c = source.get();
  if (append)
    lexeme += c;
}

int
FiltergenScanner::inspect(const int nthChar)
{
  const int c = source.get();
  if (nthChar == 0) {
    source.putback(c);
    return c;
  } else {
    const char r = inspect(nthChar - 1);
    source.putback(c);
    return r;
  }
}

bool
FiltergenScanner::skipWhitespaceAndComments()
{
  bool inComment = false;

  while (isspace(inspect()) ||
	 (inspect() == '/' && inspect(1) == '*')  ||
	 (inspect() == '#')) {

    while (isspace(inspect()))
      accept(false);

    if (inspect() == '/' && inspect(1) == '*') {
      inComment = true;
      accept(false); accept(false);

      while (inComment) {
	if (source.eof()) {
	  inComment = false;
	  return false;
	} else if (inspect() == '*' && inspect(1) == '/') {
	  accept(false); accept(false);
	  inComment = false;
	} else {
	  accept(false);
	}
      }
    }

    if (inspect() == '#') {
      inComment = true;
      accept(false);

      while (inComment) {
	if (inspect() == '\n' || source.eof()) {
	  accept(false);
	  inComment = false;
	} else {
	  accept(false);
	}
      }
    }
  }

  return true;
}

const Token
FiltergenScanner::nextToken()
{
  if (!skipWhitespaceAndComments())
    return Token::ERROR;

  lexeme.clear();

  if (source.eof())
    return Token::EOS;

  /* punctuation */
  switch (inspect()) {
  case '{':
    accept();
    return Token::LCURLY;
  case '}':
    accept();
    return Token::RCURLY;
  case '[':
    accept();
    return Token::LSQUARE;
  case ']':
    accept();
    return Token::RSQUARE;
  case ';':
    accept();
    return Token::SEMI;
  case '/':
    accept();
    return Token::SLASH;
  case ':':
    accept();
    return Token::COLON;
  case '!':
    accept();
    return Token::BANG;
  }

  /* string identifiers */
  if (inspect() == '"') {
    accept(false);
    while (!source.eof() && inspect() != '"')
      accept();
    /* TODO(jaq): should raise an error, unterminated string */
    if (source.eof())
      return Token::ERROR;
    accept(false);
    return Token::ID;
  }

  /* keywords and identifiers */
  if (isalnum(inspect())) {
    accept();
    while (isalnum(inspect()) || inspect() == '.') {
      accept();
    }
    if (keywords.find(lexeme) != keywords.end())
      return *keywords[lexeme];
    return Token::ID;
  }

  accept();
  return Token::ERROR;
}

Token *
FiltergenScanner::getToken()
{
  Token * tok = new Token(nextToken());

  return tok;
}

#if 0
void include_file(const char *);

%x include

include      BEGIN(include);

<include>[ \t]*       /* eat whitespace after include */
<include>[^ \t\n;]+    { /* include file name */
    char * name;

    name = strdup(yytext);
    include_file(name);
    free(name);

    BEGIN(INITIAL);
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

#endif
