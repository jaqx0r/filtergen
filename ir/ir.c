/* internal representation utility functions
 *
 * Copyright (c) 2005 Jamie Wilkinson <jaq@spacepants.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ir.h"
#include <stdlib.h>
#include <string.h>

#define IR_NEW(x) inline struct x##_s * x##_new() { \
	struct x##_s * x; \
	x = malloc(sizeof(struct x##_s)); \
	return x; \
    }

#define IR_FREE(x) inline void x##_free(struct x##_s * x) { \
	free(x); \
	/* FIXME: probably broken */ \
	x = 0; \
    }

IR_NEW(ir);
IR_FREE(ir);

IR_NEW(ir_chain);
IR_FREE(ir_chain);

IR_NEW(ir_rule);
IR_FREE(ir_rule);

IR_NEW(ir_action);
IR_FREE(ir_action);

IR_NEW(ir_expr);
IR_FREE(ir_expr);

IR_NEW(ir_value);
IR_FREE(ir_value);

inline
struct ir_expr_s * ir_expr_new_operator(enum ir_operator operator) {
    struct ir_expr_s * ir_expr;

    ir_expr = ir_expr_new();
    ir_expr->value = ir_value_new();
    ir_expr->value->type = IR_VAL_OPERATOR;
    ir_expr->value->u.operator = operator;

    return ir_expr;
}

inline
struct ir_expr_s * ir_expr_new_predicate(const char * predicate) {
    struct ir_expr_s * ir_expr;

    ir_expr = ir_expr_new();
    ir_expr->value = ir_value_new();
    ir_expr->value->type = IR_VAL_PREDICATE;
    ir_expr->value->u.predicate = strdup(predicate);

    return ir_expr;
}

inline
struct ir_expr_s * ir_expr_new_literal(const char * literal) {
    struct ir_expr_s * ir_expr;

    ir_expr = ir_expr_new();
    ir_expr->value = ir_value_new();
    ir_expr->value->type = IR_VAL_LITERAL;
    ir_expr->value->u.literal = strdup(literal);

    return ir_expr;
}
	
inline
struct ir_expr_s * ir_expr_new_range() {
    struct ir_expr_s * ir_expr;

    ir_expr = ir_expr_new();
    ir_expr->value = ir_value_new();
    ir_expr->value->type = IR_VAL_RANGE;

    return ir_expr;
}
