#include <cppunit/extensions/HelperMacros.h>

#include "token.h"

class FiltergenTokenTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenTokenTest);
  CPPUNIT_TEST(testCreateEOS);
  CPPUNIT_TEST(testCreateERROR);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testCreateEOS();
  void testCreateERROR();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FiltergenTokenTest);

void
FiltergenTokenTest::setUp()
{
}

void
FiltergenTokenTest::tearDown()
{
}

void
FiltergenTokenTest::testCreateEOS()
{
  Token t(Token::EOS);

  CPPUNIT_ASSERT_EQUAL(Token::EOS, t.kind);
}

void
FiltergenTokenTest::testCreateERROR()
{
  Token t(Token::ERROR);

  CPPUNIT_ASSERT_EQUAL(Token::ERROR, t.kind);
}

