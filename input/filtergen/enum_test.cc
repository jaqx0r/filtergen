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

#include "enum.h"

class EnumTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EnumTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testNull);
  CPPUNIT_TEST(testIntCast);
  CPPUNIT_TEST(testStringCast);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testNull();
  void testIntCast();
  void testStringCast();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EnumTest);

void
EnumTest::setUp()
{
}

void
EnumTest::tearDown()
{
}

void
EnumTest::testConstructor()
{
  Enum foo("bar");

  CPPUNIT_ASSERT_EQUAL(std::string("bar"), std::string(foo.enumName));
  CPPUNIT_ASSERT_EQUAL(Enum::enum_count - 1, (unsigned int) foo.val);
}

void
EnumTest::testNull()
{
  CPPUNIT_ASSERT_EQUAL(std::string("null"), std::string(Enum::null));
}

void
EnumTest::testIntCast()
{
  CPPUNIT_ASSERT_EQUAL(Enum::null.val, (int) Enum::null);
}

void
EnumTest::testStringCast()
{
  const char * s = "null";
  CPPUNIT_ASSERT_EQUAL(*s, *(const char *) Enum::null);
  CPPUNIT_ASSERT_EQUAL(std::string(s), std::string(Enum::null));
}
