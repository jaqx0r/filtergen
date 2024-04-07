/* LCOV_EXCL_START */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "filter.h"
#include "input/filtergen/ast.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"
#include "input/sourcefile.h"
#include "input/sourcepos.h"

int indent = -2;

extern int filtergen_column;
extern int convtrace;

struct filter *convert(struct ast_s *, struct filtergen_opts *);

void emit_filter(struct filter *f) {
  int i;

  indent += 2;
  for (i = 0; i < indent; i++) {
    printf(" ");
  };
  printf("( ");
  if (!f) {
    printf("error: emit_filter called with NULL\n");
    return;
  }
  switch (f->type) {
  case F_DIRECTION:
    switch (f->u.ifinfo.direction) {
    case INPUT:
      printf("input");
      break;
    case OUTPUT:
      printf("output");
      break;
    default:
      printf("error: encountered a direction %d\n", (int)f->u.ifinfo.direction);
      break;
    }
    printf(" %s\n", f->u.ifinfo.iface);
    break;
  case F_TARGET:
    switch (f->u.target) {
    case T_ACCEPT:
      printf("accept\n");
      break;
    case DROP:
      printf("drop\n");
      break;
    case T_REJECT:
      printf("reject\n");
      break;
    case MASQ:
      printf("masq\n");
      break;
    case REDIRECT:
      printf("redirect\n");
      break;
    default:
      printf("error: encountered a target %d\n", (int)f->u.target);
      break;
    }
    break;
  case F_SOURCE:
    printf("source %s", f->u.addrs.addrstr);
    if (f->u.addrs.maskstr)
      printf("/%s", f->u.addrs.maskstr);
    printf("\n");
    break;
  case F_DEST:
    printf("dest %s", f->u.addrs.addrstr);
    if (f->u.addrs.maskstr)
      printf("/%s", f->u.addrs.maskstr);
    printf("\n");
    break;
  case F_SPORT:
    printf("sport %s", f->u.ports.minstr);
    if (f->u.ports.maxstr)
      printf(":%s", f->u.ports.maxstr);
    printf("\n");
    break;
  case F_DPORT:
    printf("dport %s", f->u.ports.minstr);
    if (f->u.ports.maxstr)
      printf(":%s", f->u.ports.maxstr);
    printf("\n");
    break;
  case F_ICMPTYPE:
    printf("icmptype %s\n", f->u.icmp);
    break;
  case F_PROTO:
    printf("proto %s\n", f->u.proto.name);
    break;
  case F_NEG:
    printf("neg\n");
    emit_filter(f->u.neg);
    break;
  case F_SIBLIST:
    printf("siblist\n");
    emit_filter(f->u.sib);
    break;
  case F_SUBGROUP:
    printf("subgroup \"%s\"\n", f->u.sub.name);
    emit_filter(f->u.sub.list);
    break;
  case F_LOG:
    if (f->u.logmsg) {
      printf("log \"%s\"\n", f->u.logmsg);
    } else {
      printf("log \"\"\n");
    }
    break;
  case F_RTYPE:
    switch (f->u.rtype) {
    case ROUTEDONLY:
      printf("forward\n");
      break;
    case LOCALONLY:
      printf("local\n");
      break;
    case F_ONEWAY:
      printf("oneway\n");
      break;
    default:
      printf("error: encountered a rtype %d\n", (int)f->u.rtype);
      break;
    }
    break;
  default:
    printf("error: encountered a token %d\n", (int)f->type);
    break;
  }

  if (f->child) {
    for (i = 0; i < indent; i++)
      printf(" ");
    printf("  child\n");
    emit_filter(f->child);
  }
  if (f->next) {
    for (i = 0; i < indent; i++)
      printf(" ");
    printf("  next\n");
    emit_filter(f->next);
  }
  /*
  for (i = 0; i < indent; i++)
      printf(" ");
  printf("  negate: %d\n", f->negate);
  */
  for (i = 0; i < indent; i++)
    printf(" ");
  printf(" )\n");
  indent -= 2;
}

int main(int argc __attribute__((unused)),
         char **argv __attribute__((unused))) {
  struct filter *f = NULL;
  struct filtergen_opts o = {AF_INET};

  char *CONVTRACE;
  struct ast_s ast;
  int r;
  char *YYDEBUGTRACE;

  YYDEBUGTRACE = getenv("YYDEBUGTRACE");
  filtergen_set_debug(YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0);
  filtergen_debug = YYDEBUGTRACE ? atoi(YYDEBUGTRACE) : 0;
  CONVTRACE = getenv("CONVTRACE");
  convtrace = CONVTRACE ? atoi(CONVTRACE) : 0;

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

  f = convert(&ast, &o);

  if (!f) {
    printf("convert returned NULL\n");
    return 1;
  }

  emit_filter(f);
  return 0;
}
/* LCOV_EXCL_STOP */
