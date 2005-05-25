#ifndef FILTERGEN_INPUT_H
#define FILTERGEN_INPUT_H

#include "ir/ir.h"
#include <stdio.h>

/** A source parser turns the input file into the Internal
    Representation, i.e. the filter * structure. */
typedef struct ir_s * source_parser(FILE * file, int resolve_names);
typedef struct filter * old_source_parser(FILE * file, int resolve_names);

old_source_parser filtergen_source_parser;
source_parser ipts_source_parser;

#endif /* FILTERGEN_INPUT_H */
