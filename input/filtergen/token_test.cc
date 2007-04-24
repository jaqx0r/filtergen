#include <cppunit/extensions/HelperMacros.h>

#include "token.h"

class FiltergenTokenTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FiltergenTokenTest);
  CPPUNIT_TEST(testCreateEOS);
  CPPUNIT_TEST(testCreateERROR);
  CPPUNIT_TEST(testKindStr);
  CPPUNIT_TEST(testOutputOperator);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testCreateEOS();
  void testCreateERROR();
  void testKindStr();
  void testOutputOperator();
  void testEquality();
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

void
FiltergenTokenTest::testKindStr()
{
  Token t(Token::EOS);

  CPPUNIT_ASSERT_EQUAL(std::string("EOS"), t.kindStr());
}


void
FiltergenTokenTest::testOutputOperator()
{
  Token t1(Token::EOS);
  Token t2(Token::ERROR);
  std::ostringstream o1("");
  std::ostringstream o2("");

  o1 << t1;
  CPPUNIT_ASSERT_EQUAL(std::string("Token(EOS)"), o1.str());
  o2 << t2;
  CPPUNIT_ASSERT_EQUAL(std::string("Token(ERROR)"), o2.str());
}

void
FiltergenTokenTest::testEquality()
{
  Token t1(Token::EOS);
  Token t2(Token::EOS);

  CPPUNIT_ASSERT(t1 == t2);
  CPPUNIT_ASSERT_EQUAL(t1, t2);
}
