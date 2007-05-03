#include "enum.h"

unsigned int Enum::enum_count = 0;

Enum::Enum(const char * name):
  val(enum_count++)
{
  enumName = name;
}

Enum::Enum(const Enum & rhs):
  val(rhs.val)
{
  enumName = rhs.enumName;
}

unsigned int
Enum::getMaxEnum()
{
  return enum_count;
}

bool
Enum::operator==(const Enum & rhs) const
{
  return (enumName == rhs.enumName);
}

bool
Enum::operator!=(const Enum & rhs) const
{
  return (enumName != rhs.enumName);
}

Enum::operator int() const
{
  return val;
}

Enum::operator const char *() const
{
  return enumName;
}

const Enum Enum::null("null");
