#include <stdio.h>
#include "filter.h"

int indent = -2;

void emit_filter(struct filter * f) {
    int i;

    indent+=2;
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
	    printf("error: encountered a direction %d\n", (int) f->u.ifinfo.direction);
	    break;
	}
	printf(" %s\n", f->u.ifinfo.iface);
	break;
      case F_TARGET:
	switch(f->u.target) {
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
	    printf("error: encountered a target %d\n", (int) f->u.target);
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
	printf("log \"%s\"\n", f->u.logmsg);
	break;
      case F_RTYPE:
	switch (f->u.rtype) {
	  case ROUTEDONLY:
	    printf("forward\n");
	    break;
	  case LOCALONLY:
	    printf("local\n");
	    break;
	  default:
	    printf("error: encountered a rtype %d\n", (int) f->u.rtype);
	    break;
	}
      default:
	printf("error: encountered a token %d\n", (int) f->type);
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

int main(int argc, char ** argv) {
    struct filter * f = NULL;

    filter_fopen(NULL);

    f = filter_parse_list();

    if (f)
	emit_filter(f);
    else {
	printf("f is NULL\n");
	return 1;
    }

    return 0;
}
