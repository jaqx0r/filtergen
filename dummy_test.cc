#include <cppunit/extensions/HelperMacros.h>

class DummyTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DummyTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DummyTest);

void
DummyTest::setUp()
{
}

void
DummyTest::tearDown()
{
}

void
DummyTest::testConstructor()
{
  //CPPUNIT_FAIL("not implemented");
}
