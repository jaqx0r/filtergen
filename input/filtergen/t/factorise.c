/* factorise test
LCOV_EXCL_START
 */

#include <stdio.h>
#include <stdlib.h>

#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcefile.h"
#include "input/sourcepos.h"

int facttrace;
extern int filtergen_column;

int nesting = 0;

void nest() {
  int i;

  for (i = 0; i < nesting; i++)
    printf(" ");
  printf("(");
  nesting++;
}

void unnest() {
  int i;

  for (i = 0; i < nesting; i++)
    printf(" ");
  printf(")\n");
  nesting--;
}

void emit_negated_specifier(struct negated_specifier_s *n);

void emit_compound_specifier(struct compound_specifier_s *n) {
  struct subrule_list_s *s;
  struct specifier_list_s *t;

  if (n->list) {
    int count = 0;

    for (s = n->list; s; s = s->subrule_list)
      count++;
    if (count > 1) {
      nest();
      printf("or\n");
    }
    for (s = n->list; s; s = s->subrule_list) {
      if (s->specifier_list) {
        int count0 = 0;

        for (t = s->specifier_list; t; t = t->list)
          count0++;
        if (count0 > 1) {
          nest();
          printf("and\n");
        }
        for (t = s->specifier_list; t; t = t->list) {
          if (t->spec)
            emit_negated_specifier(t->spec);
        }
        if (count0 > 1)
          unnest();
      }
    }
    if (count > 1)
      unnest();
  }
}

void emit_specifier(struct specifier_s *n) {
  if (n->direction) {
    if (n->direction->list) {
      struct direction_argument_list_s *l;
      int count = 0;

      for (l = n->direction->list; l; l = l->list)
        count++;
      if (count > 1) {
        nest();
        printf("or\n");
      }
      for (l = n->direction->list; l; l = l->list) {
        if (l->arg) {
          nest();
          printf("direction %s\n", l->arg->direction);
          unnest();
        }
      }
      if (count > 1)
        unnest();
    }
  } else if (n->target) {
    nest();
    printf("target\n");
    unnest();
  } else if (n->host) {
    if (n->host->list) {
      struct host_argument_list_s *l;
      int count = 0;

      for (l = n->host->list; l; l = l->list)
        count++;
      if (count > 1) {
        nest();
        printf("or\n");
      }
      for (l = n->host->list; l; l = l->list) {
        if (l->arg) {
          nest();
          printf("host %s\n", l->arg->host);
          unnest();
        }
      }
      if (count > 1)
        unnest();
    }
  } else if (n->port) {
    nest();
    printf("port\n");
    unnest();
  } else if (n->protocol) {
    nest();
    printf("protocol\n");
    unnest();
  } else if (n->icmptype) {
    nest();
    printf("icmptype\n");
    unnest();
  } else if (n->option) {
    nest();
    printf("option\n");
    unnest();
  } else if (n->chaingroup) {
    nest();
    printf("chaingroup FIXME\n");
    unnest();
  } else if (n->compound) {
    emit_compound_specifier(n->compound);
  }
}

void emit_negated_specifier(struct negated_specifier_s *n) {
  if (n->spec) {
    if (n->negated) {
      nest();
      printf("not\n");
    }
    emit_specifier(n->spec);
    if (n->negated) {
      unnest();
    }
  }
}

void emit_rule(struct rule_s *n) {
  struct specifier_list_s *s;

  if (n->list) {
    int count = 0;

    for (s = n->list; s; s = s->list)
      count++;
    if (count > 1) {
      nest();
      /* specifiers have implicit AND */
      printf("and\n");
    }

    for (s = n->list; s; s = s->list) {
      if (s->spec) {
        emit_negated_specifier(s->spec);
      }
    }
    if (count > 1)
      unnest();
  }
}

void emit_ast(struct ast_s *n) {
  struct rule_list_s *r;
  if (n->list) {
    int count = 0;

    for (r = n->list; r; r = r->list)
      count++;
    if (count > 1) {
      nest();
      /* semicolons are OR */
      printf("or\n");
    }
    for (r = n->list; r; r = r->list) {
      if (r->rule) {
        emit_rule(r->rule);
      }
    }
    if (count > 1)
      unnest();
  }
}

int main(int argc, char **argv) {
  char *FACTTRACE, *YYDEBUGTRACE;
  struct ast_s ast;
  int r;

  FACTTRACE = getenv("FACTTRACE");
  facttrace = FACTTRACE ? atoi(FACTTRACE) : 0;
  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  filtergen_set_debug(YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0);
  filtergen_debug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;

  struct sourceposition pos;
  if (argc > 1) {
    sourcefile_push(&pos, argv[1]);
  } else {
    sourcefile_push(&pos, "-");
  }
  filtergen_in = current_srcfile->f;
  filtergen_lineno = current_srcfile->lineno;
  filtergen_column = current_srcfile->column;

  r = filtergen_parse(&ast);
  if (r != 0) {
    printf("parse returned %d\n", r);
    return 1;
  }

  /* do magic */
  /*    factorise(&ast); */

  emit_ast(&ast);
  return 0;
}
/* LCOV_EXCL_STOP */
