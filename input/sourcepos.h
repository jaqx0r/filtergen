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

#ifndef FILTERGEN_INPUT_SOURCEPOS_H
#define FILTERGEN_INPUT_SOURCEPOS_H

#include <stdio.h>

struct sourceposition {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *filename;
};

struct sourcefile {
  FILE *f; // File handle being read
  char *pathname;
  int lineno;
  int column;
  struct sourcefile *next; // next on stack
};

extern struct sourcefile *current_srcfile;

/* sourcefile_push opens a new file and pushes a struct sourcefile onto the
 * current_srcfile stack. It returns true on success and false on error. */
int sourcefile_push(struct sourceposition *pos, const char *pathname);

/* sourcefile_pop removes the struct sourcefile from the top of the stack.  It
 * returns true on success and false on error. */
int sourcefile_pop();

/* Create newly allocated storage of a sourceposition from an existing temporary
 * one */
struct sourceposition *make_sourcepos(struct sourceposition *loc);

/* Returns the location of the LHS of a rule whose RHS is in *rhs[1]..rhs[N} */
void merge_sourcepos(struct sourceposition *loc,
                     struct sourceposition const *rhs, int n);

#endif /* FILTERGEN_INPUT_SOURCEPOS_H */
