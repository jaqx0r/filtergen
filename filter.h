/* $Id: filter.h,v 1.5 2001/10/06 17:22:09 matthew Exp $ */
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
	F_MASQ, F_REDIRECT,
	F_SOURCE, F_DEST, F_SPORT, F_DPORT,
	F_PROTO, 
	F_NEG, F_SIBLIST,
	F_TARGET,
	/* this must be last */
	F_FILTER_MAX,
};

/* This is basically just a parse tree */
struct filter {
	enum filtertype type;
	union {
		enum filtertype target;
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
filter_tctor __new_filter, new_filter_target;
struct filter *new_filter_neg(struct filter *sub);
struct filter *new_filter_sibs(struct filter *list);
typedef struct filter *filter_ctor(enum filtertype, const char*);
filter_ctor new_filter_device, new_filter_host, new_filter_ports;
filter_tctor new_filter_proto;

/* filter manipulations */
void filter_unroll(struct filter **f);
void filter_noneg(struct filter **f);


/* from generated lexer and parer in filterlex.l */
struct filter *filter_parse_list(void);

/* from gen.c */
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
typedef int fg_callback(const struct filterent *ent, void *misc);
int filtergen_cprod(struct filter *filter, fg_callback cb, void *misc);

/* fg-util.c */
char *strapp(char *s, const char *n);
#define strapp2(s,n1,n2) strapp(strapp(s,n1),n2)

/* various drivers */
typedef int filtergen(struct filter *filter);
filtergen fg_iptables, fg_ipchains, fg_ipfilter, fg_cisco;

#endif /* _FK_FILTER_H */
