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

  CPPUNIT_ASSERT_EQUAL(0, sp.linestart);
  CPPUNIT_ASSERT_EQUAL(0, sp.lineend);
  CPPUNIT_ASSERT_EQUAL(0, sp.colstart);
  CPPUNIT_ASSERT_EQUAL(0, sp.colend);
}

void
SourcePositionTest::testConstructorArgs()
{
  SourcePosition sp(1, 2, 3, 4);

  CPPUNIT_ASSERT_EQUAL(1, sp.linestart);
  CPPUNIT_ASSERT_EQUAL(2, sp.colstart);
  CPPUNIT_ASSERT_EQUAL(3, sp.lineend);
  CPPUNIT_ASSERT_EQUAL(4, sp.colend);
}

void
SourcePositionTest::testOutputStreamOperator()
{
  std::ostringstream os;

  SourcePosition sp(1);
  os << sp;
  CPPUNIT_ASSERT_EQUAL(std::string("1"), os.str());

  /* reset os */
  os.str("");

  SourcePosition sp1(1, 2);
  os << sp1;
  CPPUNIT_ASSERT_EQUAL(std::string("1.2"), os.str());

  os.str("");
  SourcePosition sp2(1, 2, 3, 4);
  os << sp2;
  CPPUNIT_ASSERT_EQUAL(std::string("1.2-3.4"), os.str());

  os.str("");
  SourcePosition sp3(1, 2, 1, 3);
  os << sp3;
  CPPUNIT_ASSERT_EQUAL(std::string("1.2-3"), os.str());

  os.str("");
  SourcePosition sp4(1, 0, 2, 0);
  os << sp4;
  CPPUNIT_ASSERT_EQUAL(std::string("1-2"), os.str());
}
