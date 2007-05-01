#include <cppunit/extensions/HelperMacros.h>

#include "enum.h"

class EnumTest:
public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EnumTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testNull);
  CPPUNIT_TEST(testIntCast);
  CPPUNIT_TEST(testStringCast);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testNull();
  void testIntCast();
  void testStringCast();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EnumTest);

void
EnumTest::setUp()
{
}

void
EnumTest::tearDown()
{
}

void
EnumTest::testConstructor()
{
  Enum foo("bar");

  CPPUNIT_ASSERT_EQUAL(std::string("bar"), std::string(foo.enumName));
  CPPUNIT_ASSERT_EQUAL(1, foo.val);
}

void
EnumTest::testNull()
{
  CPPUNIT_ASSERT_EQUAL(std::string("null"), std::string(Enum::null.getName()));
}

void
EnumTest::testIntCast()
{
  CPPUNIT_ASSERT_EQUAL(0, (int) Enum::null);
}

void
EnumTest::testStringCast()
{
  const char * s = "null";
  CPPUNIT_ASSERT_EQUAL(*s, *(const char *) Enum::null);
  CPPUNIT_ASSERT_EQUAL(std::string(s), std::string(Enum::null));
}
