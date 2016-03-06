#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../parser.h"

extern char * ipts_text;
extern int ipts_lineno;
int ipts_lex();
char * ipts_filename();

char *tok_map(int c) {
  char *r;
  switch (c) {
  case TOK_TABLE:
    r = strdup("table");
    break;
  case TOK_CHAIN:
    r = strdup("chain");
    break;
  case TOK_OPTION:
    r = strdup("option");
    break;
  case TOK_IDENTIFIER:
    r = strdup("identifier");
    break;
  case TOK_LSQUARE:
    r = strdup("lsquare");
    break;
  case TOK_RSQUARE:
    r = strdup("rsquare");
    break;
  case TOK_COLON:
    r = strdup("colon");
    break;
  case TOK_BANG:
    r = strdup("bang");
    break;
  case TOK_QUOTE:
    r = strdup("quote");
    break;
  case TOK_COMMIT:
    r = strdup("commit");
    break;
  case TOK_NEWLINE:
    r = strdup("newline");
    break;
  default:
    r = strdup("UNRECOGNISED");
    break;
  }
  return r;
}

extern FILE * ipts_in;

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
    int c;

    if (argc > 1) {
      ipts_in = fopen(argv[1], "r");
    }

    while ((c = ipts_lex())) {
      fprintf(stderr, "%s:%d: kind = %s", ipts_filename(), ipts_lineno, tok_map(c));
      if (c == TOK_NEWLINE) {
	fprintf(stderr, "\n");
      } else {
	fprintf(stderr, ", spelling = \"%s\"\n", ipts_text);
      }
    }
    return 0;
}
