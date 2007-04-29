/* token
 *
 * Copyright (c) 2003-2007 Jamie Wilkinson <jaq@spacepants.org>
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
#include "token.h"

Token::Token(Kind initial_kind):
  kind(initial_kind)
{
  kind_names[EOS] = "EOS";
  kind_names[ERROR] = "ERROR";
  kind_names[LCURLY] = "LCURLY";
  kind_names[RCURLY] = "RCURLY";
  kind_names[LSQUARE] = "LSQUARE";
  kind_names[RSQUARE] = "RSQUARE";
  kind_names[SEMI] = "SEMI";
  kind_names[SLASH] = "SLASH";
  kind_names[COLON] = "COLON";
  kind_names[BANG] = "BANG";
  kind_names[ID] = "ID";
}

const std::string &
Token::kindStr() const
{
  std::map<Kind, std::string>::const_iterator iter = kind_names.find(kind);
  if (iter != kind_names.end())
    return (*iter).second;
  throw 0;
}

std::ostream &
operator<<(std::ostream & os, const Token & token)
{
  os << "Token(" << token.kindStr() << ")";
  return os;
}

bool
Token::operator==(const Token & other) const
{
  return this->kind == other.kind;
}

bool
Token::operator!=(const Token & other) const
{
  return this->kind != other.kind;
}

