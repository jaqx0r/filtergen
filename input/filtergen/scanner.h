/* input scanner for the filtergen language
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

#ifndef __FILTERGEN_SCANNER_H__
#define __FILTERGEN_SCANNER_H__

#include <iostream>

/** Token scanner for the filtergen language */
class FiltergenScanner
{
 public:
  /** Create a new scanner object
   * @param source the stream used as input to the scanner
   */
  FiltergenScanner(std::istream & source);

  /** Return the next token from the source
   * @return int pretending to be an identifier for the token
   */
  int nextToken();

 private:
  /** Accept the next character from the source into the current token
   * The next character is removed from the stream.
   */
  void accept();
  /** Inspect the next character in the source stream
   * Does not remove this character from the stream.
   * @return the next character in the stream
   */
  int inspect();

  /** The source file stream */
  std::istream & source;
  /** The current spelling of the current token */
  std::string currentSpelling;

  /** Unit test class requiring friend access to private attributes */
  friend class FiltergenScannerTest;
};

#endif /* ! __FILTERGEN_SCANNER_H__ */
