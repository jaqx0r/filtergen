#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include "parser.h"

class FiltergenParserTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenParserTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testSimpleRule);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testSimpleRule();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FiltergenParserTest);

void
FiltergenParserTest::setUp()
{
}

void
FiltergenParserTest::tearDown()
{
}

void
FiltergenParserTest::testConstructor()
{
  std::istringstream i("");
  FiltergenParser parser(i);

  CPPUNIT_ASSERT_EQUAL(true, parser.check());
}

void
FiltergenParserTest::testSimpleRule()
{
  std::istringstream i("# parse a very simple rule\n"
		       "accept;");
  FiltergenParser parser(i);

  CPPUNIT_ASSERT_EQUAL(true, parser.check());
}
