#ifndef FILTERGEN_INPUT_H
#define FILTERGEN_INPUT_H

#include "input/filtergen/token.h"

class Scanner
{
 public:
  virtual ~Scanner() = 0;

  /** Return the next token from the stream.
   * @return Token
   */
  virtual Token * getToken() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

#include "ir/ir.h"
#include <stdio.h>

/** A source parser turns the input file into the Internal
    Representation, i.e. the filter * structure. */
typedef struct ir_s * source_parser(FILE * file, int resolve_names);

source_parser filtergen_source_parser;
source_parser ipts_source_parser;

#ifdef __cplusplus
};
#endif

#endif /* FILTERGEN_INPUT_H */
