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

Token::Token(const char * name):
  Enum::Enum(name)
{
}

std::ostream &
operator<<(std::ostream & os, const Token & token)
{
  os << "Token(" << (const char *) token << ")";
  return os;
}

const Token Token::EOS("EOS");
const Token Token::ERROR("ERROR");
const Token Token::LCURLY("LCURLY");
const Token Token::RCURLY("RCURLY");
const Token Token::LSQUARE("LSQUARE");
const Token Token::RSQUARE("RSQUARE");
const Token Token::SEMI("SEMI");
const Token Token::SLASH("SLASH");
const Token Token::COLON("COLON");
const Token Token::BANG("BANG");
const Token Token::ID("ID");
const Token Token::ACCEPT("ACCEPT");
const Token Token::DEST("DEST");
const Token Token::DPORT("DPORT");
const Token Token::DROP("DROP");
const Token Token::FORWARD("FORWARD");
const Token Token::ICMPTYPE("ICMPTYPE");
const Token Token::INPUT("INPUT");
const Token Token::LOCAL("LOCAL");
const Token Token::LOG("LOG");
const Token Token::MASQ("MASQ");
const Token Token::ONEWAY("ONEWAY");
const Token Token::OUTPUT("OUTPUT");
const Token Token::PROTO("PROTO");
const Token Token::PROXY("PROXY");
const Token Token::REDIRECT("REDIRECT");
const Token Token::REJECT("REJECT");
const Token Token::SOURCE("SOURCE");
const Token Token::SPORT("SPORT");
const Token Token::TEXT("TEXT");
