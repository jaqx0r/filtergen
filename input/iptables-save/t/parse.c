#include <stdio.h>
#include <stdlib.h>
#include "../ast.h"

extern char * yytext;
int yyparse(void *);
extern int yydebug;
int yyrestart(FILE *);

int main(int argc, char ** argv) {
    char * YYDEBUGTRACE;
    struct ast_s ast;

    YYDEBUGTRACE = getenv("YYDEBUGTRACE");
    yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    if (argc > 1) {
        FILE * f = fopen(argv[1], "r");
        yyrestart(f);
    }

    yyparse(&ast);

    return 0;
}

