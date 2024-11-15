/* utility functions for filter generation
 *
 * Copyright (c) 2002 Matthew Kirkwood
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

#ifndef FILTERGEN_FG_UTIL_H_
#define FILTERGEN_FG_UTIL_H_

// Append n to s, returning a new string.
char *strapp(char *s, const char *n);

#define strapp2(s, n1, n2) strapp(strapp(s, n1), n2)

// Convert a string to an integer, returning zero for success and non-zero if an
// error occurred.
int str_to_int(const char *s, int *i);

#endif // FILTERGEN_FG_UTIL_H_
