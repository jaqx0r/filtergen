#include <stdio.h>
#include <stdlib.h>
#include "input/iptables-save/ast.h"
#include "input/iptables-save/parser.h"
#include "input/iptables-save/scanner.h"

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  char *YYDEBUGTRACE;
  struct ast_s ast;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  ipts_debug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

  if (argc > 1) {
    FILE *f = fopen(argv[1], "r");
    ipts_restart(f);
  } else {
    ipts_restart(stdin);
  }

  ipts_parse(&ast);

  return 0;
}
