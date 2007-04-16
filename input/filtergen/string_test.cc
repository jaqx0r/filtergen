#include <cppunit/extensions/HelperMacros.h>

class StringStreamTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StringStreamTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringStreamTest);

void
StringStreamTest::setUp()
{
}

void
StringStreamTest::tearDown()
{
}

void
StringStreamTest::testConstructor()
{
  std::string test_string("foo bar baz");
  std::istringstream s(test_string);

  CPPUNIT_ASSERT_EQUAL((int) 'f', s.get());
}
