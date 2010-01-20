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

#include "enum.h"

unsigned int Enum::enum_count = 0;

Enum::Enum(const char * name):
  val(enum_count++)
{
  enumName = name;
}

Enum::Enum(const Enum & rhs):
  val(rhs.val)
{
  enumName = rhs.enumName;
}

unsigned int
Enum::getMaxEnum()
{
  return enum_count;
}

bool
Enum::operator==(const Enum & rhs) const
{
  return (enumName == rhs.enumName);
}

bool
Enum::operator!=(const Enum & rhs) const
{
  return (enumName != rhs.enumName);
}

Enum::operator int() const
{
  return val;
}

Enum::operator const char *() const
{
  return enumName;
}

const Enum Enum::null("null");
