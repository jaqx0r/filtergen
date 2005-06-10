#include "output/output.h"
#include "ir/ir.h"
#include <assert.h>
#include <stdlib.h>

void * gv_emit_expr(struct ir_expr_s * ir_expr, FILE * f) {
    void * p;
    
    assert(ir_expr);

    if (ir_expr->value) {
	switch (ir_expr->value->type) {
	  case IR_VAL_OPERATOR:
	    switch (ir_expr->value->u.operator) {
	      case IR_OP_AND:
		fprintf(f, "\"%p\" [label=AND];", ir_expr);
		break;
	      case IR_OP_OR:
		fprintf(f, "\"%p\" [label=OR];", ir_expr);
		break;
	      case IR_OP_NOT:
		fprintf(f, "\"%p\" [label=NOT];", ir_expr);
		break;
	      default:
		fprintf(stderr, "warning: can't handle operator type %d\n", ir_expr->value->u.operator);
		abort();
	    }
	    break;
	  case IR_VAL_PREDICATE:
	    fprintf(f, "\"%p\" [label=\"%s()\"];", ir_expr, ir_expr->value->u.predicate);
	    break;
	  case IR_VAL_LITERAL:
	    fprintf(f, "\"%p\" [label=\"%s\"];", ir_expr, ir_expr->value->u.literal);
	    break;
	  case IR_VAL_RANGE:
	    fprintf(f, "\"%p\" [label=range];", ir_expr);
	    break;
	  case IR_VAL_CHAIN:
	    fprintf(f, "\"%p\" [label=\"chain expr\"];", ir_expr);
	    fprintf(f, "\"%p\" -> \"%p\" [label=chain];", ir_expr, ir_expr->value->u.chain);
	    break;
	  default:
	    fprintf(stderr, "warning: unknown expression value type\n");
	    fprintf(f, "unknown");
	}
    } else {
	fprintf(stderr, "warning: value undefined in current expression node\n");
	fprintf(f, "undef");
    }
    
    if (ir_expr->left) {
	p = gv_emit_expr(ir_expr->left, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=left];\n", ir_expr, p);
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
	abort();
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

struct ir_chain_s * gv_emit_chain(struct ir_chain_s * ir_chain, FILE * f) {
    void * p;

    fprintf(f, "\"%p\" [label=\"", ir_chain);
    if (ir_chain->name)
	fprintf(f, "%s", ir_chain->name);
    else
	fprintf(f, "(nameless)");
    fprintf(f, "\"];\n");
    if (ir_chain->rule) {
	p = gv_emit_rule(ir_chain->rule, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=\"rule\"];\n", ir_chain, p);
    }
    if (ir_chain->next) {
	p = gv_emit_chain(ir_chain->next, f);
	fprintf(f, "\"%p\" -> \"%p\" [label=\"next\"];\n", ir_chain, p);
    }
    return ir_chain;
}

void gv_emit_ir(struct ir_s * ir, FILE * f) {
    void * p;
  
    fprintf(f, "digraph ir {\n");

    fprintf(f, "ir;\n");
    
    if (ir->filter) {
	p = gv_emit_chain(ir->filter, f);
	fprintf(f, "\"ir\" -> \"%p\" [label=filter];\n", p);
    }
    
    if (ir->nat) {
	p = gv_emit_chain(ir->nat, f);
	fprintf(f, "\"ir\" -> \"%p\" [label=nat];\n", p);
    }
    
    if (ir->mangle) {
	p = gv_emit_chain(ir->mangle, f);
	fprintf(f, "\"mangle\" -> \"%p\" [label=mangle];\n", p);
    }
    
    fprintf(f, "}\n");
}

int graphviz_target_emitter(struct ir_s * ir, FILE * f) {
    assert(ir);
    gv_emit_ir(ir, f);
    return 0;
}
