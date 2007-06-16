#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include "parser.h"
#include "scanner.h"
#include "token.h"


class MockScanner:
  public FiltergenScanner
{
public:
  MockScanner();
  Token * getToken();
};

class FiltergenParserTest:
  public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenParserTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST(testSimpleRule);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testMatch();
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
  FiltergenScanner scanner(i);
  FiltergenParser parser(scanner);
}

void
FiltergenParserTest::testMatch()
{
  std::istringstream i(";");
  FiltergenScanner s(i);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_EQUAL(true, parser.match(Token::SEMI));
}

void
FiltergenParserTest::testSimpleRule()
{
  std::istringstream i("# parse a very simple rule\n"
		       "accept;");
  FiltergenScanner s(i);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_EQUAL(true, parser.parse());
}
