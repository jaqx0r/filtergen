/*
 */

#ifndef __FILTERGEN_SCANNER_H__
#define __FILTERGEN_SCANNER_H__

#include <iostream>

/** Token scanner for the filtergen language */
class FiltergenScanner
{
 public:
  /** Create a new scanner object
   * @param source the stream used as input to the scanner
   */
  FiltergenScanner(std::istream & source);

  /** Return the next token from the source
   * @return int pretending to be an identifier for the token
   */
  int nextToken();

 private:
  /** Accept the next character from the source into the current token
   * The next character is removed from the stream.
   */
  void accept();
  /** Inspect the next character in the source stream
   * Does not remove this character from the stream.
   * @return the next character in the stream
   */
  int inspect();

  /** The source file stream */
  std::istream & source;
  /** The current spelling of the current token */
  std::string currentSpelling;

  /** Unit test class requiring friend access to private attributes */
  friend class FiltergenScannerTest;
};

#endif /* ! __FILTERGEN_SCANNER_H__ */
