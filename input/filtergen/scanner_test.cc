#include <cppunit/extensions/HelperMacros.h>

#include "scanner.h"
#include "token.h"

class FiltergenScannerTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenScannerTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testAccept);
  CPPUNIT_TEST(testInspect);
  CPPUNIT_TEST(testInspectNext);
  CPPUNIT_TEST(testNextTokenEmptyStream);
  CPPUNIT_TEST(testSkipWhitespace);
  CPPUNIT_TEST(testCComment);
  CPPUNIT_TEST(testShellComment);
  CPPUNIT_TEST(testScanPunctuation);
  CPPUNIT_TEST(testScanNumbers);
  CPPUNIT_TEST(testScanKeywords);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testNextTokenEmptyStream();
  void testAccept();
  void testInspect();
  void testInspectNext();
  void testSkipWhitespace();
  void testCComment();
  void testShellComment();
  void testScanPunctuation();
  void testScanNumbers();
  void testScanKeywords();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FiltergenScannerTest);

void
FiltergenScannerTest::setUp()
{
}

void
FiltergenScannerTest::tearDown()
{
}

void
FiltergenScannerTest::testConstructor()
{
  std::istringstream i("");
  FiltergenScanner scanner(i);

  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testAccept()
{
  std::istringstream i("a");
  FiltergenScanner scanner(i);

  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
  scanner.accept();
  CPPUNIT_ASSERT_EQUAL(std::string("a"), scanner.lexeme);
}

void
FiltergenScannerTest::testInspect()
{
  std::istringstream i("a");
  FiltergenScanner scanner(i);

  CPPUNIT_ASSERT_EQUAL((int) 'a', scanner.inspect());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
  scanner.accept();
  CPPUNIT_ASSERT_EQUAL(std::string("a"), scanner.lexeme);
}

void
FiltergenScannerTest::testInspectNext()
{
  std::istringstream i("ab");
  FiltergenScanner scanner(i);

  CPPUNIT_ASSERT_EQUAL((int) 'b', scanner.inspect(1));
  CPPUNIT_ASSERT_EQUAL((int) 'a', scanner.inspect());
}

void
FiltergenScannerTest::testNextTokenEmptyStream()
{
  std::istringstream i("");
  FiltergenScanner scanner(i);

  // force empty stream
  scanner.accept();
  CPPUNIT_ASSERT_EQUAL(Token::EOS, scanner.nextToken());
}

void
FiltergenScannerTest::testSkipWhitespace()
{
  std::istringstream i(" ");
  FiltergenScanner scanner(i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source.eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testCComment()
{
  std::istringstream i("/* c comment */");
  FiltergenScanner scanner(i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source.eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testShellComment()
{
  std::istringstream i("# shell comment\n");
  FiltergenScanner scanner(i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source.eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testScanPunctuation()
{
  std::istringstream i("{}[];/:!");
  FiltergenScanner scanner(i);

  CPPUNIT_ASSERT_EQUAL(Token::LCURLY, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::RCURLY, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::LSQUARE, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::RSQUARE, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::SEMI, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::SLASH, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::COLON, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::BANG, scanner.nextToken());
}

void
FiltergenScannerTest::testScanNumbers()
{
  std::istringstream i("37 69 255");
  FiltergenScanner scanner(i);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("37"), scanner.lexeme);
  scanner.skipWhitespaceAndComments();
  scanner.lexeme.clear();
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("69"), scanner.lexeme);
  scanner.skipWhitespaceAndComments();
  scanner.lexeme.clear();
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("255"), scanner.lexeme);
}

void
FiltergenScannerTest::testScanKeywords()
{
  typedef std::vector<std::pair<Token::Kind, std::string> > keyword_t;

  keyword_t keywords;

  keywords.push_back(std::pair<Token::Kind, std::string>(Token::ACCEPT, "accept"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::DEST, "dest"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::DPORT, "dport"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::DROP, "drop"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::FORWARD, "forward"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::ICMPTYPE, "icmptype"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::INPUT, "input"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::LOCAL, "local"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::LOG, "log"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::MASQ, "masq"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::ONEWAY, "oneway"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::OUTPUT, "output"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::PROTO, "proto"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::PROXY, "proxy"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::REDIRECT, "redirect"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::REJECT, "reject"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::SOURCE, "source"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::SPORT, "sport"));
  keywords.push_back(std::pair<Token::Kind, std::string>(Token::TEXT, "text"));

  for (keyword_t::iterator it = keywords.begin();
       it != keywords.end();
       ++it) {
    std::istringstream i(it->second);
    FiltergenScanner scanner(i);

    CPPUNIT_ASSERT_EQUAL(it->first, scanner.nextToken());
  }
}

