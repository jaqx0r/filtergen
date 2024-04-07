/* LCOV_EXCL_START */
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcefile.h"
#include "input/sourcepos.h"
#include <stdlib.h>

extern int filtergen_column;

int main(int argc, char **argv) {
  char *YYDEBUGTRACE;
  struct ast_s ast;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");

  // Scanner
  filtergen_set_debug(YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0);
  // Parser
  filtergen_debug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

  struct sourceposition pos;
  if (argc > 1) {
    sourcefile_push(&pos, argv[1]);
  } else {
    sourcefile_push(&pos, "-");
  }
  filtergen_in = current_srcfile->f;
  filtergen_lineno = current_srcfile->lineno;
  filtergen_column = current_srcfile->column;

  filtergen_parse(&ast);

  return 0;
}
/* LCOV_EXCL_STOP */
