/* test input file emitter, should spit out identical (syntactically)
   as the input */

#include <stdio.h>
#include "../parser.h"
#include "../ast.h"

YYSTYPE yyparse(void);

void emit_rule_list(struct rule_list_s * rule_list) {
    printf("\n");
};

int main(int argc, char ** argv) {
    YYSTYPE r;

    r = yyparse();
    emit_rule_list(r.u_rule_list);

    return 0;
}
