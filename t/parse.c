#include <stdlib.h>

extern char * yytext;
int yyparse();
extern int yydebug;

/* void emit(); */

int main(int argc, char ** argv) {
    int c;
    char * YYDEBUGTRACE;

    YYDEBUGTRACE = getenv("YYDEBUGTRACE");
    yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    c = yyparse();

    return 0;
}

