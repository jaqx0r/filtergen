#include <stdio.h>
#include <stdarg.h>
#include "input/filtergen/parser.h"
#include "input/sourcefile.h"

// Define these before including scanner-generated stuff if needed
#define YYSTYPE FILTERGEN_STYPE
#define YYLTYPE FILTERGEN_LTYPE

struct sourcefile *current_srcfile = NULL;

void filter_error(YYLTYPE *locp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}
void include_file(YYLTYPE *locp, char *name) {}
int sourcefile_pop(void) { return 0; }

struct TestCase {
    const char *name;
    const char *input;
};

// Scanner function declarations
extern void filtergen__scan_string(const char *str);
extern int filtergen_lex(YYSTYPE *lvalp, YYLTYPE *llocp);
extern void filtergen_lex_destroy(void);

void run_test(struct TestCase test) {
    printf("Running test: %s (input: '%s')\n", test.name, test.input);
    filtergen__scan_string(test.input);
    
    YYSTYPE lvalp;
    YYLTYPE llocp;
    
    // Set dummy sourcefile
    struct sourcefile s;
    s.pathname = "test";
    current_srcfile = &s;
    
    while (1) {
        int token = filtergen_lex(&lvalp, &llocp);
        if (token == 0) break;
        printf("Got token: %d\n", token);
    }
    printf("Test finished.\n");
    filtergen_lex_destroy();
}

int main() {
    struct TestCase tests[] = {
        {"C-style comment", "/* comment */ accept;"},
        {"Nested comment", "/* /* nested */ */ accept;"},
        {"Hash comment", "# comment\naccept;"},
    };
    for (int i = 0; i < 3; i++) {
        run_test(tests[i]);
    }
    return 0;
}
