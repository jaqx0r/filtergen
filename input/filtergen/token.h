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

/** Token */
class Token
{
 public:

  /** Kinds of tokens that will be returned by the scanner. */
  enum Kind {
    ERROR = -1, /** scanner error */
    EOS = 0, /** end of stream */
  };

  /** Create a new token.
   * @param kind the kind of token being created
   */
  Token(Kind kind);

 private:
  /** The kind of token. */
  Kind kind;

  /** Unit test class requiring friend access to private attributes. */
  friend class FiltergenTokenTest;
};

#endif /* ! __FILTERGEN_TOKEN_H__ */
