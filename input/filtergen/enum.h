#ifndef _ENUM_H_
#define _ENUM_H_

class Enum
{
 public:
  Enum(const char * name);
  Enum(const Enum & rhs);

  static unsigned int getMaxEnum();

  /** Equality comparison operator. */
  bool operator==(const Enum & rhs) const;
  /** Inequality comparison operator. */
  bool operator!=(const Enum & rhs) const;

  /** Cast to int operator. */
  operator int() const;
  /** Cast to string operator. */
  operator const char*() const;

  const static Enum null;

 private:
  /** Total number of enums of this type. */
  static unsigned int enum_count;
  /** Integer value of this enum. */
  int val;
  /** String representation of this enum. */
  const char * enumName;

  friend class EnumTest;
};

#endif // ! _TYPESAFE_ENUM_H
