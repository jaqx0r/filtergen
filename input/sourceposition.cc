/* filtergen
 *
 * Copyright (c) 2003-2010 Jamie Wilkinson <jaq@spacepants.org>
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

#include "sourceposition.h"

#include <string>

SourcePosition::SourcePosition(std::string _filename,
			       int _linestart, int _colstart,
			       int _lineend, int _colend):
  filename(_filename),
  linestart(_linestart), colstart(_colstart),
  lineend(_lineend), colend(_colend)
{
}

SourcePosition::SourcePosition(int _linestart, int _colstart,
			       int _lineend, int _colend):
  filename("(none)"),
  linestart(_linestart), colstart(_colstart),
  lineend(_lineend), colend(_colend)
{
}

std::ostream &
operator<<(std::ostream & os, const SourcePosition & sp)
{
  os << sp.filename << ":" << sp.linestart;
  if (sp.colstart > 0)
    os << "." << sp.colstart;
  if (sp.lineend > 0 || sp.colend > 0) {
    os << "-";
    if (sp.lineend > 0 && sp.lineend != sp.linestart) {
      os << sp.lineend;
      if (sp.colend > 0)
	os << "." << sp.colend;
    } else {
      os << sp.colend;
    }
  }
  return os;
}
