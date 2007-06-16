#include <cppunit/extensions/HelperMacros.h>

#include "parser.h"

class FiltergenParserTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenParserTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
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
  FiltergenParser parser();
}
