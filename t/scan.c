#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../parser.h"

extern char * yytext;
int yylex();
long int lineno();
char * filename();

char * tok_map(int c) {
    char * r;
    switch (c) {
      case TOK_ACCEPT:
	r = strdup("accept"); break;
      case TOK_DEST:
	r = strdup("dest"); break;
      case TOK_DPORT:
	r = strdup("dport"); break;
      case TOK_DROP:
	r = strdup("drop"); break;
      case TOK_FORWARD:
	r = strdup("forward"); break;
      case TOK_ICMPTYPE:
	r = strdup("icmptype"); break;
      case TOK_INPUT:
	r = strdup("input"); break;
      case TOK_LCURLY:
	r = strdup("lcurly"); break;
      case TOK_LOCAL:
	r = strdup("local"); break;
      case TOK_LOG:
	r = strdup("log"); break;
      case TOK_LSQUARE:
	r = strdup("lsquare"); break;
      case TOK_MASQ: 
	r = strdup("masq"); break;
      case TOK_ONEWAY:
	r = strdup("oneway"); break;
      case TOK_OUTPUT:
	r = strdup("output"); break;
      case TOK_PROTO:
	r = strdup("proto"); break;
      case TOK_PROXY:
	r = strdup("proxy"); break;
      case TOK_RCURLY:
	r = strdup("rcurly"); break;
      case TOK_REDIRECT:
	r = strdup("redirect"); break;
      case TOK_REJECT:
	r = strdup("reject"); break;
      case TOK_RSQUARE:
	r = strdup("rsquare"); break;
      case TOK_SEMICOLON:
	r = strdup("semicolon"); break;
      case TOK_SOURCE:
	r = strdup("source"); break;
      case TOK_SPORT:
	r = strdup("sport"); break;
      case TOK_TEXT:
	r = strdup("text"); break;
      case TOK_IDENTIFIER:
	r = strdup("identifier"); break;
      case TOK_DOT:
	r = strdup("dot"); break;
      case TOK_SLASH:
	r = strdup("slash"); break;
      case TOK_BANG:
	r = strdup("bang"); break;
      case TOK_COLON:
	r = strdup("colon"); break;
      default:
	r = strdup("UNRECOGNISED"); break;
    }
    return r;
}  

int main(int argc __attribute__((unused)), char ** argv __attribute__((unused))) {
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

