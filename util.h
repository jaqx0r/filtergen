/* utility functions for filter output
 *
 * Copyright (c) 2001 Matthew Kirkwood
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

#ifndef	_FILTER_UTIL_H
#define	_FILTER_UTIL_H	1

#define	APP(l,s)	(l = strapp(l,s))
#define	APP2(l,s1,s2)	(l = strapp2(l,s1,s2))
#define	APPS(l,s)	APP2(l, " ", s)
#define	APPS2(l,s1,s2)	(APPS(l,s1), APP(l,s2))
#define	APPSS2(l,s1,s2)	(APPS(l,s1), APPS(l,s2))

#define	NEG(t)		(ent->whats_negated & (1<<F_ ## t))
#define	NEGA(l,t)	(NEG(t) ? APPS(l, "!") : l)

#endif /* _FILTER_UTIL_H */
