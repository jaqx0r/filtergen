#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../parser.h"

extern char * ipts_text;
extern int ipts_lineno;
int ipts_lex();
char * ipts_filename();

#define tok(x) case TOK_IPTS_##x: r = strdup(#x); break
char * tok_map(int c) {
    char * r;
    switch (c) {
	tok(TABLE);
	tok(CHAIN);
	tok(CHAIN_APPEND);
	tok(PROTOCOL);
	tok(SOURCE);
	tok(DESTINATION);
	tok(JUMP);
	tok(IN_INTERFACE);
	tok(OUT_INTERFACE);
	tok(FRAGMENT);
	tok(SET_COUNTERS);
	tok(MATCH);
	tok(DPORT);
	tok(SPORT);
	tok(SYN);
	tok(STATE);
	tok(ICMP_TYPE);
	tok(TCP_FLAGS);
	tok(TCP_OPTION);
	tok(MSS);
	tok(SOURCE_PORTS);
	tok(DESTINATION_PORTS);
	tok(PORTS);
	tok(TO_PORTS);
	tok(TO_SOURCE);
	tok(LIMIT);
      case TOK_IDENTIFIER:
	r = strdup("identifier"); break;
      case TOK_LSQUARE:
	r = strdup("lsquare"); break;
      case TOK_RSQUARE:
	r = strdup("rsquare"); break;
      case TOK_COLON:
	r = strdup("colon"); break;
      case TOK_BANG:
	r = strdup("bang"); break;
      case TOK_QUOTE:
	r = strdup("quote"); break;
      case TOK_COMMIT:
	r = strdup("commit"); break;
      default:
	r = strdup("UNRECOGNISED"); break;
    }
    return r;
}  

extern FILE * ipts_in;

int main(int argc, char ** argv) {
    int c;

    if (argc > 1) {
      ipts_in = fopen(argv[1], "r");
    }

    while ((c = ipts_lex())) {
      fprintf(stderr, "%s:%d: kind = %s, spelling = \"%s\"\n", ipts_filename(), ipts_lineno, tok_map(c), ipts_text);
    }
    return 0;
}

