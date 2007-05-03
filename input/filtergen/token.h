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

#ifndef __FILTERGEN_TOKEN_H__
#define __FILTERGEN_TOKEN_H__

#include <iostream>
#include <map>

#include "enum.h"

/** Token. */
class Token:
public Enum
{
 public:

  /** Constructor taking the human-visible name of the token. */
  Token(const char *);

  /** Kinds of tokens that will be returned by the scanner. */
  static const Token ERROR; /**< scanner error */
  static const Token EOS; /**< end of stream */
  static const Token LCURLY;
  static const Token RCURLY;
  static const Token LSQUARE;
  static const Token RSQUARE;
  static const Token SEMI;
  static const Token SLASH;
  static const Token COLON;
  static const Token BANG;
  static const Token ID; /**< represents identifiers */

  static const Token ACCEPT; /**< keywords follow */
  static const Token DEST;
  static const Token DPORT;
  static const Token DROP;
  static const Token FORWARD;
  static const Token ICMPTYPE;
  static const Token INPUT;
  static const Token LOCAL;
  static const Token LOG;
  static const Token MASQ;
  static const Token ONEWAY;
  static const Token OUTPUT;
  static const Token PROTO;
  static const Token PROXY;
  static const Token REDIRECT;
  static const Token REJECT;
  static const Token SOURCE;
  static const Token SPORT;
  static const Token TEXT;

  /** Output stream operator. */
  friend std::ostream & operator<<(std::ostream & os, const Token & token);

 private:
  /** Unit test class requiring friend access to private attributes. */
  friend class FiltergenTokenTest;
};

#endif /* ! __FILTERGEN_TOKEN_H__ */
