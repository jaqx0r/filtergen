#include <stdlib.h>
#include "../ast.h"

extern char * yytext;
int yyparse(void *);
extern int yydebug;

int main(int argc, char ** argv) {
    char * YYDEBUGTRACE;
    struct ast_s ast;

    YYDEBUGTRACE = getenv("YYDEBUGTRACE");
    yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    yyparse(&ast);

    return 0;
}

