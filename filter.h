/* $Id: filter.h,v 1.16 2002/08/20 22:54:38 matthew Exp $ */
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
	OPENBRACE, CLOSEBRACE,		/* lexer use only */
	OPENBRACKET, CLOSEBRACKET,
	SEMICOLON, STRING,
	INCLUDE,
	LOCALONLY, ROUTEDONLY,		/* for F_RTYPE */
	F_INPUT, F_OUTPUT,
	F_ACCEPT, F_DROP, F_REJECT,
	F_MASQ, F_REDIRECT,
	F_SOURCE, F_DEST, F_SPORT, F_DPORT,
	F_ICMPTYPE,
	F_PROTO, 
	F_NEG, F_SIBLIST, F_SUBGROUP,
	F_LOG,
	F_TARGET,
	F_RTYPE,
	/* this must be last */
	F_FILTER_MAX,
};

/* Structures which appear in both the parse tree and the output rule */
struct proto_spec {
	int num;
	char *name;
};

struct addr_spec {
	struct in_addr addr, mask;
	char *addrstr, *maskstr;
};

struct port_spec {
	int min, max;
	char *minstr, *maxstr;
};

/* This is basically just a parse tree */
struct filter {
	enum filtertype type;
	union {
		enum filtertype target;
		enum filtertype log;
		enum filtertype rtype;
		char *iface;
		struct addr_spec addrs;
		struct port_spec ports;
		char *icmp;
		struct proto_spec proto;
		struct filter *neg;
		struct filter *sib;
		struct {
			char *name;
			struct filter *list;
		} sub;
	} u;
	struct filter *child, *next;

	/* infernal use only */
	int negate;
};


/* from filter.c */
typedef struct filter *filter_tctor(enum filtertype);
filter_tctor __new_filter, new_filter_target, new_filter_log, new_filter_rtype;
struct filter *new_filter_neg(struct filter *sub);
struct filter *new_filter_sibs(struct filter *list);
struct filter *new_filter_subgroup(char *name, struct filter *list);
typedef struct filter *filter_ctor(enum filtertype, const char*);
filter_ctor new_filter_device, new_filter_host, new_filter_ports, new_filter_icmp, new_filter_proto;

/* filter manipulations */
void filter_unroll(struct filter **f);
void filter_nogroup(struct filter *f);
void filter_noneg(struct filter **f);
void filter_apply_flags(struct filter *f, long flags);


/* from generated lexer and parer in filterlex.l */
struct filter *filter_parse_list(void);

/* from gen.c */
struct filterent {
	/* Either direction+iface or groupname must be set */
	enum filtertype direction;
	char *iface;
	char *groupname;

	/* One of these must be set */
	enum filtertype target;
	char *subgroup;

	/* These may or may not be set */
	int whats_set:F_FILTER_MAX;
	int whats_negated:F_FILTER_MAX;
	struct addr_spec srcaddr, dstaddr;

	enum filtertype rtype;
	struct proto_spec proto;
	enum filtertype log;


	/* We need this not to be a union, for error-checking reasons */
	struct {
		struct {
			struct port_spec src, dst;
		} ports;
		char *icmp;
	} u;
};

struct fg_misc {
		int flags;
		void *misc;
};
typedef int fg_cb_rule(const struct filterent *ent, struct fg_misc *misc);
typedef int fg_cb_group(const char *name);
typedef struct {
	fg_cb_rule	*rule;
	fg_cb_group	*group;
} fg_callback;
int filtergen_cprod(struct filter *filter, fg_callback *cb, struct fg_misc *misc);

/* fg-util.c */
char *strapp(char *s, const char *n);
#define strapp2(s,n1,n2) strapp(strapp(s,n1),n2)
int str_to_int(const char *s, int *i);
char *int_to_str_dup(int i);

/* various drivers */
typedef int filtergen(struct filter *filter, int flags);
filtergen fg_iptables, fg_ipchains, fg_ipfilter, fg_cisco;
/* ("flags" arguments) */
#define	FF_NOSKEL	(1 << 0)	/* omit any "skeleton" rules */
#define	FF_LSTATE	(1 << 1)	/* lightweight state matching */
#define	FF_LOCAL	(1 << 2)	/* assume packets are local only */
#define	FF_ROUTE	(1 << 3)	/* assume packets are forwarded */
#define	FF_LOOKUP	(1 << 4)	/* translate host and service names into
					 * IP addresses and port numbers */

/* filtergen.c */
int oputs(const char *s);
int oprintf(const char *fmt, ...);

#endif /* _FK_FILTER_H */
