#ifndef FILTERGEN_ERROR_H
#define FILTERGEN_ERROR_H

#include "input/sourcepos.h"

void filter_error(struct sourceposition *pos, const char *fmt, ...);

#endif /* FILTERGEN_ERROR_H */
