/* Filter output callback routines.
 *
 * Copyright (c) 2002 Matthew Kirkwood
 * Copyright (c) 2003,2004 Jamie Wilkinson
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

#include "oputs.h"

#include <stdarg.h>
#include <stdio.h>

static FILE *outfile;

int open_outfile(const char *ofn) {
  if (ofn) {
    // TODO: open a different tempfile and rename on success
    outfile = fopen(ofn, "w");
    if (!outfile) {
      return 0;
    }
  } else {
    outfile = stdout;
  }
  return 1;
}

void close_outfile() {
  if (outfile) {
    fclose(outfile);
  }
}

int oputs(const char *s) {
  int r = 0;
  if (s) {
    r = fputs(s, outfile);
    if (r > 0) {
      fputc('\n', outfile);
      r++;
    }
  }
  return r;
}

int oprintf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return vfprintf(outfile, fmt, args);
}
