#include <stdlib.h>
#include "../filter.h"

extern char * yytext;
struct filter * yyparse(void);
extern int yydebug;

/* void emit(); */

int main(int argc, char ** argv) {
    struct filter * f;
    char * YYDEBUGTRACE;

    YYDEBUGTRACE = getenv("YYDEBUGTRACE");
    yydebug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

    f = yyparse();

    return 0;
}

