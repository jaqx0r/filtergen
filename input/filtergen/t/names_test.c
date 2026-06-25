#include <stdio.h>
#include "input/sourcefile.h"
#include "input/sourcepos.h"
#include "input/filtergen/parser.h"

extern int filtergen_lex(FILTERGEN_STYPE*, FILTERGEN_LTYPE*, void*);
extern void* filtergen__scan_string(const char*);
extern void filtergen__delete_buffer(void*);

struct TestCase {
    const char *names;
    int expected_token;
};

int run_test(struct TestCase *test) {
    printf("Running test for names: %s\n", test->names);
    
    struct sourceposition pos;
    sourcefile_push(&pos, "-");

    void *buffer = filtergen__scan_string(test->names);

    FILTERGEN_STYPE yylval;
    FILTERGEN_LTYPE yylloc;
    int kind = filtergen_lex(&yylval, &yylloc, NULL);
    
    int failed = 0;
    if (kind != test->expected_token) {
        printf("  Mismatch: expected %d, got %d\n", test->expected_token, kind);
        failed = 1;
    } else {
        printf("  Matched: %d\n", kind);
    }
    
    filtergen__delete_buffer(buffer);
    sourcefile_pop();
    return failed;
}

int main(int argc, char **argv) {
    struct TestCase tests[] = {
        {"foo", TOK_IDENTIFIER},
        {"bar", TOK_IDENTIFIER},
        {"baz", TOK_IDENTIFIER},
        {"quux", TOK_IDENTIFIER},
        {"mail.example.com", TOK_IDENTIFIER},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int all_passed = 1;
    for (int i = 0; i < num_tests; i++) {
        if (run_test(&tests[i])) all_passed = 0;
    }
    
    return all_passed ? 0 : 1;
}
