/* $Id: filter.h,v 1.1 2001/09/25 17:22:39 matthew Exp $ */
#ifndef _FK_FILTER_H
#define _FK_FILTER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/*
 * If it doesn't begin with "F_", it's a simple token, not an
 * filter type
 */
enum filtertype {
	YYEOF = 0,
	OPENBRACE, CLOSEBRACE,
	SEMICOLON, STRING,
	INCLUDE,
	TCP, UDP, ICMP,
	F_INPUT, F_OUTPUT,
	F_ACCEPT, F_DROP, F_REJECT,
	F_SOURCE, F_DEST, F_SPORT, F_DPORT,
	F_PROTO, 
	F_NEG, F_SIBLIST,
	/* this must be last */
	F_FILTER_MAX,
};

/* This is basically just a parse tree */
struct filter {
	enum filtertype type;
	union {
		char *iface;
		char *addrs;
		char *ports;
		enum filtertype proto;
		struct filter *neg;
		struct filter *sib;
	} u;
	struct filter *child, *next;

	/* infernal use only */
	int negate;
};


/* from filter.c */
typedef struct filter *filter_tctor(enum filtertype);
filter_tctor __new_filter;
#define new_filter_sense(x) __new_filter((x))
struct filter *new_filter_neg(struct filter *sub);
struct filter *new_filter_sibs(struct filter *list);
typedef struct filter *filter_ctor(enum filtertype, const char*);
filter_ctor new_filter_device, new_filter_host, new_filter_ports;
filter_tctor new_filter_proto;
void filter_unroll(struct filter **f);

/* from generated lexer and parer in filterlex.l */
struct filter *filter_parse_list(void);

/* from filtergen.c */
struct filterent {
	/* All these must be set */
	enum filtertype direction;
	enum filtertype target;
	char *iface;
	/* These may or may not be set */
	int whats_set:F_FILTER_MAX;
	int whats_negated:F_FILTER_MAX;
	char *srcaddr, *dstaddr;

	enum filtertype proto;
	union {
		struct {
			char *src, *dst;
		} ports;
		/* XXX - icmp types here */
	} u;
};
typedef int filtergen(const struct filterent *ent, void *misc);
int filtergen_cprod(struct filter *filter, filtergen cb, void *misc);

/* various drivers */
int fg_iptables(const struct filterent *ent, void *misc);
int fg_ios(const struct filterent *ent, void *misc);

#endif /* _FK_FILTER_H */
