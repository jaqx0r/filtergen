/*
 */

#ifndef __FILTERGEN_SCANNER_H__
#define __FILTERGEN_SCANNER_H__

#include <iostream>

/** Token scanner for the filtergen language */
class FiltergenScanner
{
 public:
  FiltergenScanner(std::istream & source);

  /** Return the next token from the source */
  int nextToken();

 private:
  /** Accept the next character from the source into the current token */
  void accept();
  /** Inspect the next character in the source stream */
  int inspect();

  /** The source file stream */
  std::istream & source;
  /** The current spelling of the current token */
  std::string currentSpelling;

  /** Unit test class requiring friend access to private attributes */
  friend class FiltergenScannerTest;
};

#endif /* ! __FILTERGEN_SCANNER_H__ */
