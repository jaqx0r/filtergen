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

IR_NEW(ir_value);
IR_FREE(ir_value);
