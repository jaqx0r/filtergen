#include <cppunit/extensions/HelperMacros.h>

#include "scanner.h"

class FiltergenScannerTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenScannerTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testNextTokenEmptyStream);
  CPPUNIT_TEST(testAccept);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testNextTokenEmptyStream();
  void testAccept();
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
  FiltergenScanner * scanner = new FiltergenScanner(i);

  CPPUNIT_ASSERT_ASSERTION_FAIL(CPPUNIT_ASSERT(0 == scanner));
}

void
FiltergenScannerTest::testNextTokenEmptyStream()
{
  std::istringstream i("");
  FiltergenScanner * scanner = new FiltergenScanner(i);

  CPPUNIT_ASSERT_EQUAL(0, scanner->nextToken());
}

void
FiltergenScannerTest::testAccept()
{
  std::istringstream i("a");
  FiltergenScanner * scanner = new FiltergenScanner(i);

  scanner->accept();
  CPPUNIT_ASSERT_EQUAL(std::string("a"), scanner->currentSpelling);
}
