#include "ir.h"
#include <stdio.h>
#include <string.h>

#define EMIT(x) void emit_##x(struct x##_s *x)
#define eprint(x)                                                              \
  {                                                                            \
    int i;                                                                     \
    for (i = 0; i < indent; i++)                                               \
      printf(" ");                                                             \
    printf("%s\n", x);                                                         \
  }

int indent = 0;
int indentsize = 1;

EMIT(ir_expr) {
  if (ir_expr) {
    eprint("(expr");
    indent += indentsize;

    switch (ir_expr->type) {
    case IR_EXPR_VALUE:
      eprint(ir_expr->value);
      break;
    case IR_EXPR_UNARY:
      switch (ir_expr->operator) {
      case IR_OP_NOT:
        eprint("(not");
        indent += indentsize;
        emit_ir_expr(ir_expr->left);
        indent -= indentsize;
        eprint(")");
        break;
      case IR_OP_PRED:
        eprint("(pred");
        eprint(ir_expr->value);
        indent += indentsize;
        emit_ir_expr(ir_expr->left);
        indent -= indentsize;
        eprint(")");
        break;
      default:
        break;
      }
      break;
    case IR_EXPR_BINARY:
      switch (ir_expr->operator) {
      case IR_OP_AND:
        eprint("(and");
        indent += indentsize;
        emit_ir_expr(ir_expr->left);
        emit_ir_expr(ir_expr->right);
        indent -= indentsize;
        eprint(")");
        break;
      case IR_OP_OR:
        eprint("(or");
        indent += indentsize;
        emit_ir_expr(ir_expr->left);
        emit_ir_expr(ir_expr->right);
        indent -= indentsize;
        eprint(")");
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }

    indent -= indentsize;
    eprint(")");
  }
}

EMIT(ir_action) {
  if (ir_action) {
    eprint("(action");
    indent += indentsize;

    switch (ir_action->type) {
    case IR_ACCEPT:
      eprint("accept");
      break;
    case IR_DROP:
      eprint("drop");
      break;
    case IR_REJECT:
      eprint("reject");
      break;
    case IR_LOG:
      eprint("log");
      break;
    default:
      /* blow up */
      break;
    }

    if (ir_action->options) {
      eprint(ir_action->options);
    }

    indent -= indentsize;
    eprint(")");
  }
}

EMIT(ir_rule) {
  if (ir_rule) {
    eprint("(rule");
    indent += indentsize;
    if (ir_rule->expr) {
      emit_ir_expr(ir_rule->expr);
    }
    if (ir_rule->action) {
      emit_ir_action(ir_rule->action);
    }

    if (ir_rule->next) {
      emit_ir_rule(ir_rule->next);
    }

    indent -= indentsize;
    eprint(")");
  }
}

EMIT(ir) {
  if (ir) {
    eprint("(ir");
    indent += indentsize;
    if (ir->filter) {
      emit_ir_rule(ir->filter);
    }
    indent -= indentsize;
    eprint(")");
  }
}

int main(void) {
  struct ir_s *ir;

  ir = ir_new();

  ir->filter = ir_rule_new();

  ir->filter->expr = ir_expr_new();
  ir->filter->action = ir_action_new();
  ir->filter->action->type = IR_ACCEPT;
  ir->filter->action->options = strdup("bim bam bom");
  ir->filter->expr->type = IR_EXPR_BINARY;
  ir->filter->expr->operator= IR_OP_AND;
  ir->filter->expr->left = ir_expr_new();
  ir->filter->expr->right = ir_expr_new();

  ir->filter->next = ir_rule_new();
  ir->filter->next->action = ir_action_new();
  ir->filter->next->action->type = IR_DROP;
  ir->filter->next->expr = ir_expr_new();

  emit_ir(ir);

  return 0;
}
