#include <stdlib.h>
#include "../tokens.h"

extern char * yytext;
int yyparse();

/* void emit(); */

extern int yydebug;

int main(int argc, char ** argv) {
    int c;
    char * YYDEBUGTRACE;

    YYDEBUGTRACE = getenv("YYDEBUGTRACE");
    yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    c = yyparse();

    return 0;
}

