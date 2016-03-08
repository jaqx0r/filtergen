#include <stdio.h>
#include <stdlib.h>
#include "../ast.h"
#include "../parser.h"

extern char *yytext;
extern int yydebug;
int yyrestart(FILE *);

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  char *YYDEBUGTRACE;
  struct ast_s ast;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

  if (argc > 0) {
    FILE *f = fopen(argv[1], "r");
    yyrestart(f);
  }

  ipts_parse(&ast);

  return 0;
}
