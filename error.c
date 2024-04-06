#include "error.h"

#include <stdarg.h>
#include <stdio.h>

void filter_error(struct sourceposition *pos, const char *fmt, ...);

void __attribute__((__format__(__printf__, 2, 3)))
filter_error(struct sourceposition *pos, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  if (pos && pos->first_line) {
    fprintf(stderr, "%s:%d.%d-%d.%d: error: ", pos->filename, pos->first_line,
            pos->first_column, pos->last_line, pos->last_column);
  }
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}
