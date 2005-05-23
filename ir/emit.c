#include "ir.h"
#include <stdio.h>
#include <string.h>

#define EMIT(x) void emit_##x(struct x##_s * x)
#define eprint(x) {int i;for (i=0;i<indent;i++) printf(" "); printf("%s\n", x);}

int indent = 0;
int indentsize = 1;

EMIT(ir_node) {
  if (ir_node) {
    eprint("(node");
    indent += indentsize;

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
    if (ir_rule->predicates) {
      emit_ir_node(ir_rule->predicates);
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
    indent+=indentsize;
    if (ir->filter) {
      emit_ir_rule(ir->filter);
    }
    indent-=indentsize;
    eprint(")");
  }
}

int main(void) {
    struct ir_s * ir;

    ir = ir_new();

    ir->filter = ir_rule_new();

    ir->filter->predicates = ir_node_new();
    ir->filter->action = ir_action_new();
    ir->filter->action->type = IR_ACCEPT;
    ir->filter->action->options = strdup("bim bam bom");

    ir->filter->next = ir_rule_new();
    ir->filter->next->action = ir_action_new();
    ir->filter->next->action->type = IR_DROP;

    emit_ir(ir);

    return 0;
}
