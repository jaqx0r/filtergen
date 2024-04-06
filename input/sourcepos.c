/* source position functions
 *
 * Copyright (c) 2013 Jamie Wilkinson <jaq@spacepants.org>
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

#include "sourcepos.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Top of source file stack during parse. */
struct sourcefile *current_srcfile = NULL;

/* Limit infinite recursion. */
#define MAX_SRCFILE_DEPTH 100
static int srcfile_depth = 0;

int sourcefile_push(struct sourceposition *pos, const char *pathname) {
  struct sourcefile *sf;

  if (srcfile_depth++ > MAX_SRCFILE_DEPTH) {
    filter_error(pos,
                 "too many nested includes.  skipping include of file %s\n",
                 pathname);
    return 0;
  }

  if ((sf = malloc(sizeof(*sf))) == NULL) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "malloc failed attempting to push new sourcefile onto "
                    "stack when opening %s: %s\n",
            pathname, strerror(errno));
    return 0;
    /* LCOV_EXCL_STOP */
  }

  if (pathname == NULL || strncmp(pathname, "-", 1) == 0) {
    sf->f = stdin;
    if (asprintf(&sf->pathname, "<stdin>") < 0) {
      /* LCOV_EXCL_START */
      fprintf(stderr, "error: asprintf allocation failed when constructing "
                      "sourcefile pathname for %s\n",
              pathname);
      free(sf);
      return 0;
      /* LCOV_EXCL_STOP */
    }
  } else {
    sf->f = fopen(pathname, "r");
    if (!sf->f) {
      fprintf(stderr, "fopen failed reading %s: %s\n", pathname,
              strerror(errno));
      free(sf);
      return 0;
    }
    if (asprintf(&sf->pathname, "%s", pathname) < 0) {
      /* LCOV_EXCL_START */
      fprintf(stderr, "error: asprintf allocation failed when constructing "
                      "sourcefile pathname for %s\n",
              pathname);
      free(sf);
      return 0;
      /* LCOV_EXCL_STOP */
    }
  }
  sf->next = current_srcfile;
  sf->lineno = 1;
  sf->column = 1;

  current_srcfile = sf;
  return 1;
}

int sourcefile_pop() {
  struct sourcefile *sf = current_srcfile;

  current_srcfile = sf->next;

  if (fclose(sf->f)) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "failed to close file when popping sourcefile stack: %s\n",
            strerror(errno));
    return 0;
    /* LCOV_EXCL_STOP */
  }
  free(sf);
  srcfile_depth--;
  return 1;
}

struct sourceposition *make_sourcepos(struct sourceposition *loc) {
  struct sourceposition *pos;
  pos = malloc(sizeof(struct sourceposition));
  memcpy(pos, loc, sizeof(struct sourceposition));
  return pos;
}

void merge_sourcepos(struct sourceposition *loc,
                     struct sourceposition const *rhs, int n) {
  if (n) {
    loc->first_line = rhs[1].first_line;
    loc->first_column = rhs[1].first_column;
    loc->last_line = rhs[n].last_line;
    loc->last_column = rhs[n].last_column;
    if (rhs[1].filename) {
      loc->filename = strdup(rhs[1].filename);
    } else {
      loc->filename = NULL;
    }
  } else {
    loc->first_line = loc->last_line = rhs[0].last_line;
    loc->first_column = loc->last_column = rhs[0].last_column;
    if (rhs[0].filename) {
      loc->filename = strdup(rhs[0].filename);
    } else {
      loc->filename = NULL;
    }
  }
}
