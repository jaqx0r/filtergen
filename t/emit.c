/* ast emitter
 * output should be identical (syntactically) to the input
 */

#include <stdio.h>
#include "../ast.h"

int yyparse(void *);

void emit_rule(struct rule_s * rule) {
    /* rules end in a semicolon and newline */
    printf(";\n");
}

void emit_rule_list(struct rule_list_s * rule_list) {
    if (rule_list->rule_list) {
	emit_rule_list(rule_list->rule_list);
    }
    if (rule_list->rule) {
	emit_rule(rule_list->rule);
    }
};

void emit_ast(struct ast_s * ast) {
    if (ast->rule_list)
	emit_rule_list(ast->rule_list);
};

int main(int argc, char ** argv) {
    struct ast_s ast;
    int res;

    res = yyparse((void *)&ast);

    if (res != 0) {
	printf("yyparse returned %d\n", res);
	return 1;
    }

    emit_ast(&ast);

    return 0;
}
