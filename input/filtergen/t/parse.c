#include <stdlib.h>
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcepos.h"

extern int yycolumn;

int main(int argc, char **argv) {
  char *YYDEBUGTRACE;
  struct ast_s ast;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  filtergen_debug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

  if (argc > 1) {
    sourcefile_push(argv[1]);
  } else {
    sourcefile_push("-");
  }
  filtergen_in = current_srcfile->f;
  filtergen_lineno = current_srcfile->lineno;
  yycolumn = current_srcfile->column;

  filtergen_parse(&ast);

  return 0;
}
