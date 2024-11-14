/* Filter output callback routines.
 *
 * Copyright (c) 2002 Matthew Kirkwood
 * Copyright (c) 2024 Jamie Wilkinson
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

#ifndef OPUTS_H
#define OPUTS_H

#ifdef __GNUC__
#define _PRINTF_ATTR(s, e) __attribute__((__format__(__printf__, (s), (e))))
#else
#define _PRINTF_ATTR(s, e)
#endif

/** Open the filter output file used by the oputs functions. */
int open_outfile(const char *ofn);

/** Close the filter output file if opened. */
void close_outfile();

/** Output a string, and appends a newline, to the target file. */
int oputs(const char *s);

/** Write a string, printf style, to the target file. */
int oprintf(const char *fmt, ...) _PRINTF_ATTR(1, 2);

#endif /* OPUTS_H */
