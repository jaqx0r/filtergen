#ifndef __IPTS_DRIVER_H__
#define __IPTS_DRIVER_H__

#include <string>
#include "ast.h"

// Forward declarations
union YYSTYPE;

namespace yy
{
  class location;
  class ipts_parser;
}
class ipts_driver;

// announce to flex the prototype we want for lexing function
#define YY_DECL int yylex(YYSTYPE * yylval, yy::location* yylloc, ipts_driver & driver)
// and declare it for the parser's sake
YY_DECL;

// The ipts syntax input driver
class ipts_driver
{
 public:
    ipts_driver();
    virtual ~ipts_driver();

    struct ast_s * result;

    // Scanner
    void scan_begin();
    void scan_end();
    bool trace_scanning;

    // Parser
    void parse(FILE * f, const std::string & fn);
    FILE * file;
    std::string filename;
    bool trace_parsing;

    // error handling
    void error(const yy::location & l, const std::string & m);
    void error(const std::string & m);
};

#endif // ! __IPTS_DRIVER_H__
