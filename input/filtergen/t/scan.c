/* LCOV_EXCL_START */
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcepos.h"

char *tok_map(int c) {
  char *r;
  switch (c) {
  case TOK_ACCEPT:
    r = strdup("accept");
    break;
  case TOK_DEST:
    r = strdup("dest");
    break;
  case TOK_DPORT:
    r = strdup("dport");
    break;
  case TOK_DROP:
    r = strdup("drop");
    break;
  case TOK_FORWARD:
    r = strdup("forward");
    break;
  case TOK_ICMPTYPE:
    r = strdup("icmptype");
    break;
  case TOK_INPUT:
    r = strdup("input");
    break;
  case TOK_LCURLY:
    r = strdup("lcurly");
    break;
  case TOK_LOCAL:
    r = strdup("local");
    break;
  case TOK_LOG:
    r = strdup("log");
    break;
  case TOK_LSQUARE:
    r = strdup("lsquare");
    break;
  case TOK_MASQ:
    r = strdup("masq");
    break;
  case TOK_ONEWAY:
    r = strdup("oneway");
    break;
  case TOK_OUTPUT:
    r = strdup("output");
    break;
  case TOK_PROTO:
    r = strdup("proto");
    break;
  case TOK_PROXY:
    r = strdup("proxy");
    break;
  case TOK_RCURLY:
    r = strdup("rcurly");
    break;
  case TOK_REDIRECT:
    r = strdup("redirect");
    break;
  case TOK_REJECT:
    r = strdup("reject");
    break;
  case TOK_RSQUARE:
    r = strdup("rsquare");
    break;
  case TOK_SEMICOLON:
    r = strdup("semicolon");
    break;
  case TOK_SOURCE:
    r = strdup("source");
    break;
  case TOK_SPORT:
    r = strdup("sport");
    break;
  case TOK_TEXT:
    r = strdup("text");
    break;
  case TOK_IDENTIFIER:
    r = strdup("identifier");
    break;
  case TOK_DOT:
    r = strdup("dot");
    break;
  case TOK_SLASH:
    r = strdup("slash");
    break;
  case TOK_BANG:
    r = strdup("bang");
    break;
  case TOK_COLON:
    r = strdup("colon");
    break;
  case TOK_STAR:
    r = strdup("star");
    break;
  default:
    r = strdup("UNRECOGNISED");
    break;
  }
  return r;
}

extern FILE *filtergen_in;
extern int yycolumn;
extern int filtergen_lineno;

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  int c;

  char *YYDEBUGTRACE;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  filtergen_set_debug(YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0);

  struct sourceposition pos;
  if (argc > 1) {
    sourcefile_push(&pos, argv[1]);
  } else {
    sourcefile_push(&pos, "-");
  }
  YYSTYPE yylval;
  YYLTYPE yylloc;
  filtergen_in = current_srcfile->f;
  filtergen_lineno = current_srcfile->lineno;
  yycolumn = current_srcfile->column;

  /* all output is considered a compiler message by dejagnu */
  while ((c = filtergen_lex(&yylval, &yylloc))) {
    if (c == TOK_UNEXPECTED) {
      continue;
    }
    fprintf(stderr, "%s:%d: kind = %s, spelling = \"%s\"\n",
            current_srcfile->pathname, current_srcfile->lineno, tok_map(c),
            filtergen_text);
  }
  return 0;
}
/* LCOV_EXCL_STOP */
