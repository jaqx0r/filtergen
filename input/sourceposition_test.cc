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
#include <iostream>

#include "sourceposition.h"

class SourcePositionTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SourcePositionTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testConstructorArgs);
  CPPUNIT_TEST(testOutputStreamOperator);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testConstructorArgs();
  void testOutputStreamOperator();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SourcePositionTest);

void
SourcePositionTest::setUp()
{
}

void
SourcePositionTest::tearDown()
{
}

void
SourcePositionTest::testConstructor()
{
  SourcePosition sp;

  CPPUNIT_ASSERT_EQUAL(std::string("(none)"), sp.filename);
  CPPUNIT_ASSERT_EQUAL(0, sp.linestart);
  CPPUNIT_ASSERT_EQUAL(0, sp.lineend);
  CPPUNIT_ASSERT_EQUAL(0, sp.colstart);
  CPPUNIT_ASSERT_EQUAL(0, sp.colend);
}

void
SourcePositionTest::testConstructorArgs()
{
  SourcePosition sp("foo", 1, 2, 3, 4);

  CPPUNIT_ASSERT_EQUAL(std::string("foo"), sp.filename);
  CPPUNIT_ASSERT_EQUAL(1, sp.linestart);
  CPPUNIT_ASSERT_EQUAL(2, sp.colstart);
  CPPUNIT_ASSERT_EQUAL(3, sp.lineend);
  CPPUNIT_ASSERT_EQUAL(4, sp.colend);
}

void
SourcePositionTest::testOutputStreamOperator()
{
  std::ostringstream os;

  SourcePosition sp("foo", 1);
  os << sp;
  CPPUNIT_ASSERT_EQUAL(std::string("foo:1"), os.str());

  /* reset os */
  os.str("");

  SourcePosition sp1("bar", 1, 2);
  os << sp1;
  CPPUNIT_ASSERT_EQUAL(std::string("bar:1.2"), os.str());

  os.str("");
  SourcePosition sp2(1, 2, 3, 4);
  os << sp2;
  CPPUNIT_ASSERT_EQUAL(std::string("(none):1.2-3.4"), os.str());

  os.str("");
  SourcePosition sp3(1, 2, 1, 3);
  os << sp3;
  CPPUNIT_ASSERT_EQUAL(std::string("(none):1.2-3"), os.str());

  os.str("");
  SourcePosition sp4("quux", 1, 0, 2, 0);
  os << sp4;
  CPPUNIT_ASSERT_EQUAL(std::string("quux:1-2"), os.str());
}
