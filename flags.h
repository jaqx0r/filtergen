/* flags
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

#ifndef FLAGS_H
#define FLAGS_H

/** ("flags" arguments) */
enum flags {
  FF_NOSKEL = (1 << 0),    /* omit any "skeleton" rules */
  FF_LSTATE = (1 << 1),    /* lightweight state matching */
  FF_LOCAL = (1 << 2),     /* assume packets are local only */
  FF_ROUTE = (1 << 3),     /* assume packets are forwarded */
  FF_NORESOLVE = (1 << 4), /* don't resolve hostnames, ports, or services */
  FF_FLUSH = (1 << 5),     /* just flush the ruleset instead */
};

#endif /* FLAGS_H */
