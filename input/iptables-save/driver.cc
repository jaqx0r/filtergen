#include "driver.h"
#include "parser.hh"

ipts_driver::ipts_driver()
    : trace_scanning(false), trace_parsing(false)
{
}

ipts_driver::~ipts_driver()
{
}

void
ipts_driver::parse(FILE * f, const std::string & fn)
{
    file = f;
    filename = fn;
    scan_begin();
    yy::ipts_parser parser(*this);
    parser.set_debug_level(trace_parsing);
    parser.parse();
    scan_end();
}

void
ipts_driver::error(const yy::location & l,
			const std::string & m)
{
    std::cerr << l << ": " << m << std::endl;
}

void
ipts_driver::error(const std::string & m)
{
    std::cerr << m << std::endl;
}
