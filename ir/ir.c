#include "ir.h"
#include <stdlib.h>

#define IR_NEW(x) struct x##_s * x##_new() { \
	struct x##_s * x; \
	x = malloc(sizeof(struct x##_s)); \
	return x; \
    }

#define IR_FREE(x) void x##_free(struct x##_s * x) { \
	free(x); \
	/* FIXME: probably broken */ \
	x = 0; \
    }

IR_NEW(ir);
IR_FREE(ir);

IR_NEW(ir_rule);
IR_FREE(ir_rule);

IR_NEW(ir_action);
IR_FREE(ir_action);

IR_NEW(ir_expr);
IR_FREE(ir_expr);
