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
  CPPUNIT_TEST(testCCommentNoEnd);
  CPPUNIT_TEST(testShellComment);
  CPPUNIT_TEST(testShellCommentNoEOL);
  CPPUNIT_TEST(testScanPunctuation);
  CPPUNIT_TEST(testScanNumbers);
  CPPUNIT_TEST(testScanKeywords);
  CPPUNIT_TEST(testScanNames);
  CPPUNIT_TEST(testScanNetworkNames);
  CPPUNIT_TEST(testInterspersedComments);
  CPPUNIT_TEST(testStringIdentifier);
  CPPUNIT_TEST(testStringIdentifierNoEnd);
  CPPUNIT_TEST(testNextTokenCommentEOF);
  CPPUNIT_TEST(testInclude);
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
  void testCCommentNoEnd();
  void testShellComment();
  void testShellCommentNoEOL();
  void testScanPunctuation();
  void testScanNumbers();
  void testScanKeywords();
  void testScanNames();
  void testScanNetworkNames();
  void testInterspersedComments();
  void testStringIdentifier();
  void testStringIdentifierNoEnd();
  void testNextTokenCommentEOF();
  void testInclude();
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
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
  CPPUNIT_ASSERT_EQUAL((size_t) 1, scanner.sources.size());
}

void
FiltergenScannerTest::testAccept()
{
  std::istringstream i("a");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
  scanner.accept();
  CPPUNIT_ASSERT_EQUAL(std::string("a"), scanner.lexeme);
}

void
FiltergenScannerTest::testInspect()
{
  std::istringstream i("a");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL((int) 'a', scanner.inspect());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
  scanner.accept();
  CPPUNIT_ASSERT_EQUAL(std::string("a"), scanner.lexeme);
}

void
FiltergenScannerTest::testInspectNext()
{
  std::istringstream i("ab");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL((int) 'b', scanner.inspect(1));
  CPPUNIT_ASSERT_EQUAL((int) 'a', scanner.inspect());
}

void
FiltergenScannerTest::testNextTokenEmptyStream()
{
  std::istringstream i("");
  FiltergenScanner scanner(&i);

  // force empty stream
  scanner.accept();
  CPPUNIT_ASSERT_EQUAL(Token::EOS, scanner.nextToken());
}

void
FiltergenScannerTest::testSkipWhitespace()
{
  std::istringstream i(" ");
  FiltergenScanner scanner(&i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testCComment()
{
  std::istringstream i("/* c comment */");
  FiltergenScanner scanner(&i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testShellComment()
{
  std::istringstream i("# shell comment\n");
  FiltergenScanner scanner(&i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testShellCommentNoEOL()
{
  std::istringstream i("# shell comment no EOL");
  FiltergenScanner scanner(&i);

  scanner.skipWhitespaceAndComments();
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testScanPunctuation()
{
  std::istringstream i("{}[];/:!");
  FiltergenScanner scanner(&i);

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
  FiltergenScanner scanner(&i);

  /* 37 */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("37"), scanner.lexeme);
  /* 69 */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("69"), scanner.lexeme);
  /* 255 */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("255"), scanner.lexeme);
}

void
FiltergenScannerTest::testScanKeywords()
{
  typedef std::vector<std::pair<Token, std::string> > keyword_t;

  keyword_t keywords;

  keywords.push_back(std::pair<Token, std::string>(Token::ACCEPT, "accept"));
  keywords.push_back(std::pair<Token, std::string>(Token::DEST, "dest"));
  keywords.push_back(std::pair<Token, std::string>(Token::DPORT, "dport"));
  keywords.push_back(std::pair<Token, std::string>(Token::DROP, "drop"));
  keywords.push_back(std::pair<Token, std::string>(Token::FORWARD, "forward"));
  keywords.push_back(std::pair<Token, std::string>(Token::ICMPTYPE, "icmptype"));
  keywords.push_back(std::pair<Token, std::string>(Token::INPUT, "input"));
  keywords.push_back(std::pair<Token, std::string>(Token::LOCAL, "local"));
  keywords.push_back(std::pair<Token, std::string>(Token::LOG, "log"));
  keywords.push_back(std::pair<Token, std::string>(Token::MASQ, "masq"));
  keywords.push_back(std::pair<Token, std::string>(Token::ONEWAY, "oneway"));
  keywords.push_back(std::pair<Token, std::string>(Token::OUTPUT, "output"));
  keywords.push_back(std::pair<Token, std::string>(Token::PROTO, "proto"));
  keywords.push_back(std::pair<Token, std::string>(Token::PROXY, "proxy"));
  keywords.push_back(std::pair<Token, std::string>(Token::REDIRECT, "redirect"));
  keywords.push_back(std::pair<Token, std::string>(Token::REJECT, "reject"));
  keywords.push_back(std::pair<Token, std::string>(Token::SOURCE, "source"));
  keywords.push_back(std::pair<Token, std::string>(Token::SPORT, "sport"));
  keywords.push_back(std::pair<Token, std::string>(Token::TEXT, "text"));

  for (keyword_t::iterator it = keywords.begin();
       it != keywords.end();
       ++it) {
    std::istringstream i(it->second);
    FiltergenScanner scanner(&i);

    CPPUNIT_ASSERT_EQUAL(it->first, scanner.nextToken());
  }
}

void
FiltergenScannerTest::testScanNames()
{
  std::istringstream i("foo bar mail.example.com");
  FiltergenScanner scanner(&i);

  /* foo */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), scanner.lexeme);
  /* bar */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), scanner.lexeme);
  /* mail.example.com */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("mail.example.com"), scanner.lexeme);

  CPPUNIT_ASSERT_EQUAL(Token::EOS, scanner.nextToken());
}

void
FiltergenScannerTest::testScanNetworkNames()
{
  std::istringstream i("0.0.0.0 http 127.0.0.1/255.255.255.255 bar/29 "
		       "bar/255.255.255.248");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("0.0.0.0"), scanner.lexeme);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), scanner.lexeme);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("127.0.0.1"), scanner.lexeme);
  CPPUNIT_ASSERT_EQUAL(Token::SLASH, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("255.255.255.255"), scanner.lexeme);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), scanner.lexeme);
  CPPUNIT_ASSERT_EQUAL(Token::SLASH, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("29"), scanner.lexeme);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), scanner.lexeme);
  CPPUNIT_ASSERT_EQUAL(Token::SLASH, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("255.255.255.248"), scanner.lexeme);

  CPPUNIT_ASSERT_EQUAL(Token::EOS, scanner.nextToken());
}

void
FiltergenScannerTest::testInterspersedComments()
{
  std::istringstream i("/* blah */word\n"
		       "# blah\n"
		       "word\n"
		       "word #blah\n"
		       "/* c comments can't nest */ ***/\n"
		       "/* this /* is okay */word\n");
  FiltergenScanner scanner(&i);

  /* c comment */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("word"), scanner.lexeme);
  scanner.lexeme.clear();
  /* shell comment */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("word"), scanner.lexeme);
  scanner.lexeme.clear();
  /* shell comment end of line */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("word"), scanner.lexeme);
  scanner.lexeme.clear();
  /* c comments can't nest */
  CPPUNIT_ASSERT_EQUAL(Token::ERROR, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("*"), scanner.lexeme);
  scanner.lexeme.clear();
  CPPUNIT_ASSERT_EQUAL(Token::ERROR, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("*"), scanner.lexeme);
  scanner.lexeme.clear();
  CPPUNIT_ASSERT_EQUAL(Token::ERROR, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("*"), scanner.lexeme);
  scanner.lexeme.clear();
  CPPUNIT_ASSERT_EQUAL(Token::SLASH, scanner.nextToken());
  scanner.lexeme.clear();
  /* last one is ok */
  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("word"), scanner.lexeme);
  scanner.lexeme.clear();

  CPPUNIT_ASSERT_EQUAL(Token::EOS, scanner.nextToken());
}

void
FiltergenScannerTest::testCCommentNoEnd()
{
  std::istringstream i("/* a comment without"
		       "   an ending is like haiku"
		       "   with too many syllables");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(false, scanner.skipWhitespaceAndComments());
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
  CPPUNIT_ASSERT_EQUAL(std::string(""), scanner.lexeme);
}

void
FiltergenScannerTest::testStringIdentifier()
{
  std::istringstream i("\"string\"");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("string"), scanner.lexeme);
}

void
FiltergenScannerTest::testStringIdentifierNoEnd()
{
  std::istringstream i("\"string");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(Token::ERROR, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
}

void
FiltergenScannerTest::testNextTokenCommentEOF()
{
  std::istringstream i("foo /* comment"
		       "bar # comment");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), scanner.lexeme);
  CPPUNIT_ASSERT_EQUAL(Token::ERROR, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(true, scanner.source()->eof());
}

void
FiltergenScannerTest::testInclude()
{
  //TODO(jaq): this touches the filesystem :(
  std::istringstream i("include testsuite/filtergen.input/fg.scan/include");
  FiltergenScanner scanner(&i);

  CPPUNIT_ASSERT_EQUAL(Token::ID, scanner.nextToken());
  CPPUNIT_ASSERT_EQUAL(std::string("pants"), scanner.lexeme);
  CPPUNIT_ASSERT_EQUAL(Token::EOS, scanner.nextToken());
}
