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

#include <cppunit/extensions/HelperMacros.h>

#include "token.h"

class FiltergenTokenTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenTokenTest);
  CPPUNIT_TEST(testKindStr);
  CPPUNIT_TEST(testOutputOperator);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testKindStr();
  void testOutputOperator();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FiltergenTokenTest);

void
FiltergenTokenTest::setUp()
{
}

void
FiltergenTokenTest::tearDown()
{
}

void
FiltergenTokenTest::testKindStr()
{
  CPPUNIT_ASSERT_EQUAL(std::string("EOS"), std::string(Token::EOS));
}


void
FiltergenTokenTest::testOutputOperator()
{
  std::ostringstream o1("");
  std::ostringstream o2("");

  o1 << Token::EOS;
  CPPUNIT_ASSERT_EQUAL(std::string("Token(EOS)"), o1.str());
  o2 << Token::ERROR;
  CPPUNIT_ASSERT_EQUAL(std::string("Token(ERROR)"), o2.str());
}
