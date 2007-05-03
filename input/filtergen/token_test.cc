#include <cppunit/extensions/HelperMacros.h>

#include "token.h"

class FiltergenTokenTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenTokenTest);
  CPPUNIT_TEST(testKindStr);
  CPPUNIT_TEST(testOutputOperator);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testKindStr();
  void testOutputOperator();
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
FiltergenTokenTest::testKindStr()
{
  CPPUNIT_ASSERT_EQUAL(std::string("EOS"), std::string(Token::EOS));
}


void
FiltergenTokenTest::testOutputOperator()
{
  std::ostringstream o1("");
  std::ostringstream o2("");

  o1 << Token::EOS;
  CPPUNIT_ASSERT_EQUAL(std::string("Token(EOS)"), o1.str());
  o2 << Token::ERROR;
  CPPUNIT_ASSERT_EQUAL(std::string("Token(ERROR)"), o2.str());
}
