#include "sourceposition.h"

#include <string>

SourcePosition::SourcePosition(int _linestart, int _colstart,
			       int _lineend, int _colend):
  linestart(_linestart), colstart(_colstart),
 lineend(_lineend), colend(_colend)
{
}

std::ostream &
operator<<(std::ostream & os, const SourcePosition & sp)
{
  os << sp.linestart;
  if (sp.colstart > 0)
    os << "." << sp.colstart;
  if (sp.lineend > 0 || sp.colend > 0) {
    os << "-";
    if (sp.lineend > 0 && sp.lineend != sp.linestart) {
      os << sp.lineend;
      if (sp.colend > 0)
	os << "." << sp.colend;
    } else {
      os << sp.colend;
    }
  }
  return os;
}
