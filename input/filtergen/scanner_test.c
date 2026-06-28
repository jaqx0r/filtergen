#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcefile.h"
#include <stdarg.h>
#include <stdio.h>

struct TestCase {
  const char *name;
  const char *input;
  struct {
    const int token;
    const char *spelling;
  } expected[10];
};

struct sourcefile *current_srcfile = NULL;

void filter_error(YYLTYPE *locp, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}
int sourcefile_pop(void) { return 0; }

char *tok_map(int c) {
  char *r;
  switch (c) {
  case TOK_ACCEPT:
    r = strdup("accept");
    break;
  case TOK_DEST:
    r = strdup("dest");
    break;
  case TOK_DPORT:
    r = strdup("dport");
    break;
  case TOK_DROP:
    r = strdup("drop");
    break;
  case TOK_FORWARD:
    r = strdup("forward");
    break;
  case TOK_ICMPTYPE:
    r = strdup("icmptype");
    break;
  case TOK_INPUT:
    r = strdup("input");
    break;
  case TOK_LCURLY:
    r = strdup("lcurly");
    break;
  case TOK_LOCAL:
    r = strdup("local");
    break;
  case TOK_LOG:
    r = strdup("log");
    break;
  case TOK_LSQUARE:
    r = strdup("lsquare");
    break;
  case TOK_MASQ:
    r = strdup("masq");
    break;
  case TOK_ONEWAY:
    r = strdup("oneway");
    break;
  case TOK_OUTPUT:
    r = strdup("output");
    break;
  case TOK_PROTO:
    r = strdup("proto");
    break;
  case TOK_PROXY:
    r = strdup("proxy");
    break;
  case TOK_RCURLY:
    r = strdup("rcurly");
    break;
  case TOK_REDIRECT:
    r = strdup("redirect");
    break;
  case TOK_REJECT:
    r = strdup("reject");
    break;
  case TOK_RSQUARE:
    r = strdup("rsquare");
    break;
  case TOK_SEMICOLON:
    r = strdup("semicolon");
    break;
  case TOK_SOURCE:
    r = strdup("source");
    break;
  case TOK_SPORT:
    r = strdup("sport");
    break;
  case TOK_TEXT:
    r = strdup("text");
    break;
  case TOK_IDENTIFIER:
    r = strdup("identifier");
    break;
  case TOK_DOT:
    r = strdup("dot");
    break;
  case TOK_SLASH:
    r = strdup("slash");
    break;
  case TOK_BANG:
    r = strdup("bang");
    break;
  case TOK_COLON:
    r = strdup("colon");
    break;
  case TOK_STAR:
    r = strdup("star");
    break;
  default:
    r = strdup("UNRECOGNISED");
    break;
  }
  return r;
}

int run_test(struct TestCase *test) {
  printf("Running test: %s (input: '%s')\n", test->name, test->input);
  filtergen__scan_string(test->input);

  YYSTYPE lvalp;
  YYLTYPE llocp;

  // Set dummy sourcefile
  struct sourcefile s;
  s.pathname = "test";
  current_srcfile = &s;

  int i = 0;
  while (1) {
    int token = filtergen_lex(&lvalp, &llocp);
    if (token == 0)
      break;

    if (test->expected[i].token != 0) {
      if (token != test->expected[i].token) {
        printf("Error: expected token %s, got %s\n",
               tok_map(test->expected[i].token), tok_map(token));
        return -1;
      }
      int token_length = strlen(filtergen_text);
      int expected_length = strlen(test->expected[i].spelling);
      if (token_length != expected_length) {
        printf("Error: expected spelling len %d, got %d\n", expected_length,
               token_length);
        return -1;
      }
      if (strncmp(filtergen_text, test->expected[i].spelling, token_length) !=
          0) {
        printf("Error: expected spelling \"%s\", got \"%s\"\n",
               test->expected[i].spelling, filtergen_text);
        return -1;
      }
    }
    printf("Got token: %d\n", token);
    i++;
  }
  printf("Test finished.\n");
  filtergen_lex_destroy();
  return 0;
}

int main() {
  struct TestCase tests[] = {
      {
          "c-style comment",
          "/* blah */ word\r",
          {
              {TOK_IDENTIFIER, "word"},
              {0, ""},
          },
      },

      {
          "shell-style hash comment",
          "# blah\rword\r",
          {
              {TOK_IDENTIFIER, "word"},
              {0, ""},
          },
      },

      {
          "line ending with a hash comment",
          "word#blah\rwork\r",
          {
              {TOK_IDENTIFIER, "word"},
              {TOK_IDENTIFIER, "word"},
              {0, ""},
          },
      },

      {
          "nested end comment",
          "/* c comments can't nest */ ***/\r",
          {
              {TOK_STAR, "*"},
              {TOK_STAR, "*"},
              {TOK_STAR, "*"},
              {TOK_SLASH, "/"},
              {0, ""},
          },
      },

      {
          "nested start comment",
          "/* this /* is ok */word\r",
          {
              {TOK_IDENTIFIER, "word"},
              {0, ""},
          },
      },

      {
          "identifier",
          "foo",
          {
              {TOK_IDENTIFIER, "foo"},
          },
      },

      {
          "identifier with dots",
          "mail.example.com",
          {
              {TOK_IDENTIFIER, "mail.example.com"},
          },
      },

      // keywords
      {"accept", "accept", {{TOK_ACCEPT, "accept"}}},
      {"dest", "dest", {{TOK_DEST, "dest"}}},
      {"dport", "dport", {{TOK_DPORT, "dport"}}},
      {"drop", "drop", {{TOK_DROP, "drop"}}},
      {"forward", "forward", {{TOK_FORWARD, "forward"}}},
      {"icmptype", "icmptype", {{TOK_ICMPTYPE, "icmptype"}}},
      {"input", "input", {{TOK_INPUT, "input"}}},
      {"local", "local", {{TOK_LOCAL, "local"}}},
      {"log", "log", {{TOK_LOG, "log"}}},
      {"masq", "masq", {{TOK_MASQ, "masq"}}},
      {"oneway", "oneway", {{TOK_ONEWAY, "oneway"}}},
      {"output", "output", {{TOK_OUTPUT, "output"}}},
      {"proto", "proto", {{TOK_PROTO, "proto"}}},
      {"proxy", "proxy", {{TOK_PROXY, "proxy"}}},
      {"redirect", "redirect", {{TOK_REDIRECT, "redirect"}}},
      {"reject", "reject", {{TOK_REJECT, "reject"}}},
      {"source", "source", {{TOK_SOURCE, "source"}}},
      {"sport", "sport", {{TOK_SPORT, "sport"}}},
      {"text", "text", {{TOK_TEXT, "text"}}},

      {"identifiers",
       "37 69 0.0.0.0",
       {{TOK_IDENTIFIER, "37"},
        {TOK_IDENTIFIER, "69"},
        {TOK_IDENTIFIER, "0.0.0.0"}}},
      {
          "netmask",
          "127.0.0.1/255.255.255.255",
          {{TOK_IDENTIFIER, "12.0.0.1"},
           {TOK_SLASH, "/"},
           {TOK_IDENTIFIER, "255.255.255.255"}},
      },
      /* punctuation */
      {"slash", "/", {{TOK_SLASH, "/"}}},
      {"lcurly", "{", {{TOK_LCURLY, "{"}}},
      {"rcurly", "}", {{TOK_RCURLY, "}"}}},
      {"lsquare", "[", {{TOK_LSQUARE, "["}}},
      {"rsquare", "]", {{TOK_RSQUARE, "]"}}},
      {"semicolon", ";", {{TOK_SEMICOLON, ":"}}},
      {"colon", ":", {{TOK_COLON, ":"}}},
      {"exclamation", "!", {{TOK_BANG, "!"}}},
      {"asterisk", "*", {{TOK_STAR, "*"}}},
  };

  int failed = 0;
  for (int i = 0; i < 3; i++) {
    int r = run_test(&tests[i]);
    failed += r;
  }
  return failed != 0;
}
