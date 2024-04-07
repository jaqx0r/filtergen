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

#include <stdlib.h>
#include <string.h>

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
