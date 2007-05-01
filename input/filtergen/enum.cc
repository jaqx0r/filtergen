#include "enum.h"

unsigned int Enum::enum_count = 0;

const Enum Enum::null("null");

Enum::operator int() const
{
  return val;
}

Enum::operator const char *() const
{
  return enumName;
}
