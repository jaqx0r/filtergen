#ifndef _ENUM_H_
#define _ENUM_H_

class Enum
{
 private:
  static unsigned int enum_count;
  int val;
  const char * enumName;

 public:
 Enum(): val(0), enumName(0) {}
 Enum(const char * name): val(enum_count)
  {
    enumName = name;
    enum_count++;
  }

  const static Enum null;

 Enum(const Enum & rhs) : val(rhs.val)
    {
      enumName = rhs.enumName;
    }

  const char * getName() const { return enumName; }
  static unsigned int getMaxEnum() { return enum_count; }

  bool operator==(const Enum & rhs) const
  {
    return (enumName == rhs.enumName);
  }

  bool operator!=(const Enum & rhs) const
  {
    return (enumName != rhs.enumName);
  }

  /** Cast to int operator. */
  operator int() const;
  /** Cast to string operator. */
  operator const char*() const;

 private:
  friend class EnumTest;
};

#endif // ! _TYPESAFE_ENUM_H
