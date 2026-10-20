#include <string>
#include <cstdarg>
#include <vector>
#include <gtest/gtest.h>

extern "C" {
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcefile.h"
}

struct TestCase {
  std::string name;
  std::string input;
  struct {
    const int token;
    std::string spelling;
  } expected[10];
};

void filter_error(YYLTYPE *locp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int size = vsnprintf(nullptr, 0, fmt, ap);
    va_end(ap);

    if (size > 0) {
        std::vector<char> buffer(size + 1);
        va_start(ap, fmt);
        vsnprintf(buffer.data(), buffer.size(), fmt, ap);
        va_end(ap);

        FAIL() << buffer.data();
    }
}

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

class ScannerTestSuite : public testing::TestWithParam<TestCase> {
    // protected:
    //     void TearDown() override {
    //         filtergen_lex_destroy();
    //   }
};

TEST_P(ScannerTestSuite, Lex) {
  YYSTYPE lvalp;
  YYLTYPE llocp;

  filtergen_set_debug(1);

  // sourcefile_pop frees this
  current_srcfile = (struct sourcefile *)malloc(sizeof(struct sourcefile));
  ASSERT_NE(current_srcfile, nullptr);
  current_srcfile->f = NULL;
  current_srcfile->pathname = NULL;

  ASSERT_NE(nullptr, filtergen__scan_string(GetParam().input.c_str()));
  
  int i = 0;
  while (const int token = filtergen_lex(&lvalp, &llocp)) {
    ASSERT_EQ(token, GetParam().expected[i].token);
    ASSERT_EQ(filtergen_text, GetParam().expected[i].spelling);
    i++;
  }
  //EXPECT_EQ(i, std::size(GetParam().expected));
}

static const TestCase tests[] = {
    {
        "c_style_comment",
        "/* blah */ word",
        {
            {TOK_IDENTIFIER, "word"},
            {0, ""},
        },
    },

    {
        "shell_style_hash_comment",
        "# blahword",
        {
            {TOK_IDENTIFIER, "word"},
            {0, ""},
        },
    },

    {
"line_ending_with_a_hash_comment",
        "word#blahwork",
        {
            {TOK_IDENTIFIER, "word"},
            {TOK_IDENTIFIER, "word"},
            {0, ""},
        },
    },

    {
"nested_end_comment",
        "/* c comments can't nest */ ***/",
        {
            {TOK_STAR, "*"},
            {TOK_STAR, "*"},
            {TOK_STAR, "*"},
            {TOK_SLASH, "/"},
            {0, ""},
        },
    },

    {
"nested_start_comment",
        "/* this /* is ok */word",
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
"identifier_with_dots",
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
        {{TOK_IDENTIFIER, "127.0.0.1"},
         {TOK_SLASH, "/"},
         {TOK_IDENTIFIER, "255.255.255.255"}},
    },
    /* punctuation */
    {"slash", "/", {{TOK_SLASH, "/"}}},
    {"lcurly", "{", {{TOK_LCURLY, "{"}}},
    {"rcurly", "}", {{TOK_RCURLY, "}"}}},
    {"lsquare", "[", {{TOK_LSQUARE, "["}}},
    {"rsquare", "]", {{TOK_RSQUARE, "]"}}},
    {"semicolon", ";", {{TOK_SEMICOLON, ";"}}},
    {"colon", ":", {{TOK_COLON, ":"}}},
    {"exclamation", "!", {{TOK_BANG, "!"}}},
    {"asterisk", "*", {{TOK_STAR, "*"}}},
};

INSTANTIATE_TEST_SUITE_P(
    LexTests, ScannerTestSuite, testing::ValuesIn(tests),
    [](const ::testing::TestParamInfo<ScannerTestSuite::ParamType> &info) {
        return info.param.name;
    }                           
);
