#include "../tokens.h"

extern char * yytext;
int yylex();
long int lineno();
char * filename();

char * tok_map[] = {
    "",
    "accept",
    "dest",
    "dport",
    "drop",
    "forward",
    "icmptype",
    "include",
    "input",
    "lcurly",
    "local",
    "log",
    "lsquare",
    "masq",
    "output",
    "proto",
    "proxy",
    "rcurly",
    "redirect",
    "reject",
    "rsquare",
    "semicolon",
    "source",
    "sport",
    "stringliteral",
    "text",
    "",
    "name",
    "number",
    "dot",
    "slash",
    "error",
};  

int main(int argc, char ** argv) {
    int c;

    while (c = yylex()) {
	printf("kind = %d [<%s>], spelling = \"%s\", file = \"%s\", line = %ld\n", c, tok_map[c], yytext, filename(), lineno());
    }
    return 0;
}

