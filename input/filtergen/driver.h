#ifndef __FILTERGEN_DRIVER_H__
#define __FILTERGEN_DRIVER_H__

#include <string>
#include "ast.h"


/* #define YY_DECL int yylex(YYSTYPE * yylval, yy::location* yylloc, filtergen_driver & driver) */
/* // and declare it for the parser's sake */
/* YY_DECL; */

/* // The filtergen syntax input driver */
/* class filtergen_driver */
/* { */
/*  public: */
/*     filtergen_driver(); */
/*     virtual ~filtergen_driver(); */

/*     struct ast_s * result; */

/*     // Scanner */
/*     void scan_begin(); */
/*     void scan_end(); */
/*     bool trace_scanning; */

/*     // Parser */
/*     void parse(FILE * f, const std::string & fn); */
/*     FILE * file; */
/*     std::string filename; */
/*     bool trace_parsing; */

/*     // error handling */
/*     void error(const yy::location & l, const std::string & m); */
/*     void error(const std::string & m); */
/* }; */

#endif // ! __FILTERGEN_DRIVER_H__
