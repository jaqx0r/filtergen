#ifndef FILTERGEN_OUPUT_H
#define FILTERGEN_OUPUT_H

#include "ir/ir.h"
#include <stdio.h>

/** A target emitter turns the intermediate representation */
typedef int target_emitter(struct ir_s * ir, FILE * file);

target_emitter emit_filtergen;
target_emitter graphviz_target_emitter;

#endif /* FILTERGEN_OUTPUT_H */
