/* syntax tree
 */

#ifndef __AST_H__
#define __AST_H__

struct specifier_list_s {
};

struct rule_s {
    struct specifier_list_s * specifier_list;
};

struct rule_list_s {
    struct rule_list_s * rule_list;
    struct rule_s * rule;
};

struct ast_s {
    struct rule_list_s * rule_list;
};

#endif /* __AST_H__ */
