#include <stdlib.h>
#include "../filter.h"

extern char * yytext;
struct filter * yyparse(void);
extern int yydebug;

/* void emit(); */

int main(int argc, char ** argv) {
    char * YYDEBUGTRACE;

    YYDEBUGTRACE = getenv("YYDEBUGTRACE");
    yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    yyparse();

    return 0;
}

