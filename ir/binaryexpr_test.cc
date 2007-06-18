#include <cppunit/extensions/HelperMacros.h>

#include "binaryexpr.h"

class BinaryExprTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BinaryExprTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryExprTest);

void
BinaryExprTest::setUp()
{
}

void
BinaryExprTest::tearDown()
{
}

void
BinaryExprTest::testConstructor()
{
  //IR::BinaryExpr b;

  //CPPUNIT_ASSERT(b);
  //CPPUNIT_FAIL("not implemented");
}
