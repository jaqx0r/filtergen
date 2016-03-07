#include <stdlib.h>

#include "../ast.h"
#include "../../sourcepos.h"

extern char *yytext;
int yyparse(void *);
extern int yydebug;
extern char * filtergen_text;
int filtergen_lex();
int filtergen_get_lineno();
int filtergen_set_debug(int);
char * filtergen_filename();

extern FILE* filtergen_in;
extern int yycolumn;
extern int filtergen_lineno;


int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  char *YYDEBUGTRACE;
  struct ast_s ast;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    if (argc > 1) {
        sourcefile_push(argv[1]);
    } else {
        sourcefile_push("-");
    }
    filtergen_in = current_srcfile->f;
    filtergen_lineno = current_srcfile->lineno;
    yycolumn = current_srcfile->column;
    
  yyparse(&ast);

  return 0;
}
