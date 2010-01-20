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

class StringStreamTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StringStreamTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringStreamTest);

void
StringStreamTest::setUp()
{
}

void
StringStreamTest::tearDown()
{
}

void
StringStreamTest::testConstructor()
{
  std::string test_string("foo bar baz");
  std::istringstream s(test_string);

  CPPUNIT_ASSERT_EQUAL((int) 'f', s.get());
}
