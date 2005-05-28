#include "output/output.h"
#include "ir/ir.h"
#include <assert.h>

void * gv_emit_value(struct ir_value_s * ir_value, FILE * f) {
    assert(ir_value);

    fprintf(f, "\"%p\" [label=\"", ir_value);
    switch (ir_value->type) {
      case IR_VAL_OPERATOR:
	switch (ir_value->u.operator) {
	  case IR_OP_AND:
	    fprintf(f, "AND");
	    break;
	  case IR_OP_OR:
	    fprintf(f, "OR");
	    break;
	  case IR_OP_NOT:
	    fprintf(f, "NOT");
	    break;
	  default:
	    fprintf(stderr, "warning: can't handle operator type %d\n", ir_value->u.operator);
	}
	break;
      case IR_VAL_PREDICATE:
	fprintf(f, "%s()", ir_value->u.predicate);
	break;
      case IR_VAL_LITERAL:
	fprintf(f, "%s", ir_value->u.literal);
	break;
      case IR_VAL_RANGE:
	fprintf(f, ":");
	break;
      default:
	fprintf(stderr, "warning: can't emit value type %d\n", ir_value->type);
    }

    fprintf(f, "\"];\n");

    return ir_value;
}

void * gv_emit_expr(struct ir_expr_s * ir_expr, FILE * f) {
    void * p;
    
    assert(ir_expr);

    fprintf(f, "\"%p\" [label=\"", ir_expr);
    if (ir_expr->value) {
	switch (ir_expr->value->type) {
	  case IR_VAL_OPERATOR:
	    fprintf(f, "operator");
	    break;
	  case IR_VAL_PREDICATE:
	    fprintf(f, "predicate");
	    break;
	  case IR_VAL_LITERAL:
	    fprintf(f, "literal");
	    break;
	  case IR_VAL_RANGE:
	    fprintf(f, "range");
	    break;
	  default:
	    fprintf(stderr, "warning: unknown expression value type\n");
	    fprintf(f, "unknown");
	}
    } else {
	fprintf(stderr, "warning: value undefined in current expression node\n");
	fprintf(f, "undef");
    }
    fprintf(f, "\"];\n");
    
    if (ir_expr->left) {
	p = gv_emit_expr(ir_expr->left, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=left];\n", ir_expr, p);
    }

    if (ir_expr->value) {
	p = gv_emit_value(ir_expr->value, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=value];\n", ir_expr, p);
    }	

    if (ir_expr->right) {
	p = gv_emit_expr(ir_expr->right, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=right];\n", ir_expr, p);
    }

    return ir_expr;
}

void * gv_emit_action(struct ir_action_s * ir_action, FILE * f) {
    assert(ir_action);
    fprintf(f, "\"%p\" [label=\"", ir_action);
    switch (ir_action->type) {
     case IR_ACCEPT:
      fprintf(f, "ACCEPT");
      break;
     case IR_DROP:
      fprintf(f, "DROP");
      break;
     case IR_REJECT:
      fprintf(f, "REJECT");
      break;
     case IR_LOG:
      fprintf(f, "LOG");
      break;
     default:
      fprintf(stderr, "warning: unrecognised action type %d\n", ir_action->type);
    }
    fprintf(f, "\"];\n");

    if (ir_action->option) {
      struct ir_expr_s * p;
      
      p = gv_emit_expr(ir_action->option, f);
      fprintf(f, "\"%p\" -> \"%p\" [label=option];\n", ir_action, p);
    }
      
    return ir_action;
}

void * gv_emit_rule(struct ir_rule_s * ir_rule, FILE * f) {
    void * p;
    assert(ir_rule);
    fprintf(f, "\"%p\" [label=\"rule\"];\n", ir_rule);
    if (ir_rule->expr) {
	p = gv_emit_expr(ir_rule->expr, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=expr];\n", ir_rule, p);
    }
    if (ir_rule->action) {
	p = gv_emit_action(ir_rule->action, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=action];\n", ir_rule, p);
    }
    if (ir_rule->next) {
	p = gv_emit_rule(ir_rule->next, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=next];\n", ir_rule, p);
    }
    return ir_rule;
}

void gv_emit_ir(struct ir_s * ir, FILE * f) {
  struct ir_rule_s * p;
  
    fprintf(f, "digraph ir {\n");

    if (ir->filter) {
      fprintf(f, "\"filter\";\n");
      p = gv_emit_rule(ir->filter, f);
      fprintf (f, "\"filter\" -> \"%p\";\n", p);
    }
    
    if (ir->nat) {
      fprintf(f, "\"nat\";\n");
      p = gv_emit_rule(ir->nat, f);
      fprintf (f, "\"nat\" -> \"%p\";\n", p);
    }
    
    if (ir->mangle) {
      fprintf(f, "\"mangle\";\n");
      p = gv_emit_rule(ir->mangle, f);
      fprintf (f, "\"mangle\" -> \"%p\";\n", p);
    }
    
    fprintf(f, "}\n");
}

int graphviz_target_emitter(struct ir_s * ir, FILE * f) {
    assert(ir);
    gv_emit_ir(ir, f);
    return 0;
}
