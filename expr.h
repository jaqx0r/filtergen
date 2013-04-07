#ifndef __EXPR_H__
#define __EXPR_H__

enum expression = {
    EXP_AND,
    EXP_OR,
    EXP_NOT,
    EXP_PRED,
};

struct expr_s * {
    enum expression type;
};

enum action = {
    ACT_NONE,
    ACT_ACCEPT,
    ACT_REJECT,
    ACT_DROP,
};

/* rule is a boolean expression of filtering predicates -> action */
struct rule_s {
    struct expr_s * expr;
    enum action act;
    struct rule_s * next;
};

struct group_s {
    char * name;
    struct rule_s * rule_list;
};


#endif /* __EXPR_H__ */
