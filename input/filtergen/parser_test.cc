#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

#include "parser.h"
#include "token.h"

class FiltergenParserTest:
  public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenParserTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testAccept);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST(testMatchFailure);
  CPPUNIT_TEST(testParseRule);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testAccept();
  void testMatch();
  void testMatchFailure();
  void testParseRule();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FiltergenParserTest);

/** A mock scanner object that we can fill with a sequence of tokens. */
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
  MockScanner s;
  s.tokens.push_back(Token::EOS);
  FiltergenParser parser(s);
}

void
FiltergenParserTest::testAccept()
{
  MockScanner s;
  s.tokens.push_back(Token::ACCEPT);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_EQUAL(Token::ACCEPT, *(parser.currentToken));
}

void
FiltergenParserTest::testMatch()
{
  MockScanner s;
  s.tokens.push_back(Token::SEMI);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_EQUAL(true, parser.match(Token::SEMI));
}

void
FiltergenParserTest::testMatchFailure()
{
  MockScanner s;
  s.tokens.push_back(Token::SEMI);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_EQUAL(false, parser.match(Token::ACCEPT));
}

void
FiltergenParserTest::testParseRule()
{
  MockScanner s;
  s.tokens.push_back(Token::SEMI);
  FiltergenParser parser(s);

  CPPUNIT_ASSERT_NO_THROW(parser.parseRule());
}
