#include <stdlib.h>
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcepos.h"

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  char *YYDEBUGTRACE;
  struct ast_s ast;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  filtergen_debug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

  if (argc > 1) {
    sourcefile_push(argv[1]);
  } else {
    sourcefile_push("-");
  }

  filtergen_parse(&ast);

  return 0;
}
