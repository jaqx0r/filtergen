#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "../parser.h"

extern char * yytext;
int yylex();
long int lineno();
char * filename();

char * tok_map(int c) {
    char * r;
    switch (c) {
      case TOK_ACCEPT:
	r = "accept"; break;
      case TOK_DEST:
	r = "dest"; break;
      case TOK_DPORT:
	r = "dport"; break;
      case TOK_DROP:
	r = "drop"; break;
      case TOK_FORWARD:
	r = "forward"; break;
      case TOK_ICMPTYPE:
	r = "icmptype"; break;
      case TOK_INPUT:
	r = "input"; break;
      case TOK_LCURLY:
	r = "lcurly"; break;
      case TOK_LOCAL:
	r = "local"; break;
      case TOK_LOG:
	r = "log"; break;
      case TOK_LSQUARE:
	r = "lsquare"; break;
      case TOK_MASQ: 
	r = "masq"; break;
      case TOK_OUTPUT:
	r = "output"; break;
      case TOK_PROTO:
	r = "proto"; break;
      case TOK_PROXY:
	r = "proxy"; break;
      case TOK_RCURLY:
	r = "rcurly"; break;
      case TOK_REDIRECT:
	r = "redirect"; break;
      case TOK_REJECT:
	r = "reject"; break;
      case TOK_RSQUARE:
	r = "rsquare"; break;
      case TOK_SEMICOLON:
	r = "semicolon"; break;
      case TOK_SOURCE:
	r = "source"; break;
      case TOK_SPORT:
	r = "sport"; break;
      case TOK_TEXT:
	r = "text"; break;
      case TOK_IDENTIFIER:
	r = "name"; break;
      case TOK_NUMBER:
	r = "number"; break;
      case TOK_DOT:
	r = "dot"; break;
      case TOK_SLASH:
	r = "slash"; break;
      case TOK_BANG:
	r = "bang"; break;
      case TOK_COLON:
	r = "colon"; break;
      default:
	r = "UNRECOGNISED"; break;
    }
    return r;
}  

int main(int argc, char ** argv) {
    int c;

    /* if running in make distcheck the cwd isn't the same as the srcdir */
    if (getenv("srcdir")) {
	chdir(getenv("srcdir"));
    }

    while ((c = yylex())) {
	printf("kind = %s, spelling = \"%s\", file = \"%s\", line = %ld\n", tok_map(c), yytext, filename(), lineno());
    }
    return 0;
}

