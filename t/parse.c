#include "../tokens.h"

extern char * yytext;
int yyparse();

/* void emit(); */

int main(int argc, char ** argv) {
    int c;

    c = yyparse();

    return 0;
}

