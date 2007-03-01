#ifndef FILTERGEN_INPUT_H
#define FILTERGEN_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "filter.h"
#include <stdio.h>

/** A source parser turns the input file into the Internal
    Representation, i.e. the filter * structure. */
typedef struct filter * source_parser(FILE * file, char * filename, int resolve_names);

source_parser filtergen_source_parser;
source_parser ipts_source_parser;

#ifdef __cplusplus
}
#endif
    
#endif /* FILTERGEN_INPUT_H */
