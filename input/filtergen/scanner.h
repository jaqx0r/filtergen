/*
 */

#ifndef __FILTERGEN_SCANNER_H__
#define __FILTERGEN_SCANNER_H__

#include <iostream>

class FiltergenScanner
{
 public:
  FiltergenScanner(std::istream & source);

  int nextToken();

 private:
  void accept();
  int inspect();

  friend class FiltergenScannerTest;
  std::istream & source;
  std::string currentSpelling;
};

#endif /* ! __FILTERGEN_SCANNER_H__ */
