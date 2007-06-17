#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include "parser.h"
#include "scanner.h"
#include "token.h"

class FiltergenParserTest:
  public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenParserTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST(testSimpleRule);
  CPPUNIT_TEST(testParseRule);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testMatch();
  void testSimpleRule();
  void testParseRule();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FiltergenParserTest);

/** A mock scanner object that we can fill with tokens. */
class MockScanner:
  public Scanner
{
public:
  int counter;
  std::vector<Token> tokens;
  MockScanner(): counter(0) {};
  virtual Token * getToken() { return new Token(tokens[counter++]); }
};

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

void
FiltergenParserTest::testParseRule()
{
  MockScanner s;
  s.tokens.push_back(Token::SEMI);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_EQUAL(true, parser.parseRule());
}
