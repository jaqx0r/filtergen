#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../parser.h"

extern char * filtergen_text;
int filtergen_lex();
extern int filtergen_lineno;
char * filtergen_filename();

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

extern FILE * filtergen_in;

int main(int argc, char ** argv) {
    int c;

    if (argc > 1) {
	filtergen_in = fopen(argv[1], "r");
    }

    /* all output is considered a compiler message */
    while ((c = filtergen_lex())) {
	fprintf(stderr, "%s:%d: kind = %s, spelling = \"%s\"\n", filtergen_filename(), filtergen_lineno, tok_map(c), filtergen_text);
    }
    return 0;
}
