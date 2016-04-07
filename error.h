#ifndef __ERROR_H__
#define __ERROR_H__

#include "input/sourcepos.h"

void filter_error(struct sourceposition *pos, const char *fmt, ...);

#endif /* __ERROR_H__ */
