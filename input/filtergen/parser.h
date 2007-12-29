/* input parser for the filtergen language
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

#ifndef __FILTERGEN_PARSER_H__
#define __FILTERGEN_PARSER_H__

#include <iostream>

#include "input/input.h"
#include "token.h"

/** Parser for the filtergen language. */
class FiltergenParser
{
 public:
  /** Create a new parser object.
   * @param scanner the scanner used to scan the input and return tokens
   */
  FiltergenParser(Scanner & scanner);

  /** Parse the input according to the grammar of this parser.
   * @return bool
   */
  void parse();

 private:
  /** Accept the current token, and advance the "token pointer" in the source
   * stream.
   */
  void accept();

  /** Match a token in the stream.
   * @param expected the expected token
   */
  bool match(const Token & expected);

  /** Parse a rule_list production.
   *
   * \code rule_list := | rule_list rule \endcode
   */
  void parseRuleList();
  /** Parse a rule production.
   *
   * \code rule := specifier_list ';' \endcode
   */
  void parseRule();
  /** Parse a specifier_list production.
   *
   * \code specifier_list := | specifier_list negated_specifier \endcode
   */
  void parseSpecifierList();
  /** Parse a negated_specifier production.
   *
   * \code negated_specifier := specifier | '!' specifier \endcode
   */
  void parseNegatedSpecifier();
  /** Parse a specifier production.
   *
   * \code specifier := compound_specifier | direction_specifier | target_specifier | host_specifier | port_specifier | protocol_specifier | icmptype_specifier | option_specifier | chaingroup_specifier \endcode
   */
  void parseSpecifier();
  /* ... */
  /** Parse a subrule_list production.
   *
   * \code subrule_list := specifier_list | subrule_list ';' specifier_list \endcode
   */
  void parseSubruleList();
  /** Parse a chaingroup_specifier production.
   *
   * \code chaingroup_specifier := subrule_list \endcode
   */
  void parseChaingroupSpecifier();

  /** The source scanner. */
  Scanner & scanner;

  /** The current token being parsed. */
  const Token * currentToken;

  /** Unit test class requiring friend access to private attributes. */
  friend class FiltergenParserTest;
};

#endif /* ! __FILTERGEN_PARSER_H__ */
