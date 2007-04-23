#include <cppunit/extensions/HelperMacros.h>

#include "scanner.h"

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

  CPPUNIT_ASSERT_EQUAL(0, scanner.nextToken());
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
