/* types and functions used throughout filtergen
 *
 * Copyright (c) 2002 Matthew Kirkwood
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _FK_FILTER_H
#define _FK_FILTER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#ifdef	__GNUC__
#define	_PRINTF_ATTR(s,e) __attribute__ ((__format__(__printf__, (s), (e))))
#else
#define	_PRINTF_ATTR(s,e)
#endif


/*
 * If it doesn't begin with "F_", it's a simple token, not an
 * filter type
 */
enum filtertype {
    YYEOF = 0,
    F_DIRECTION, F_TARGET,
    F_SOURCE, F_DEST, F_SPORT, F_DPORT,
    F_ICMPTYPE,
    F_PROTO, 
    F_NEG, F_SIBLIST, F_SUBGROUP,
    F_LOG,
    F_RTYPE,
    F_ONEWAY,
    /* this must be the last real filter type */
    F_FILTER_MAX,
    /* parser use only */
    INPUT, OUTPUT,
    /* lex uses "ACCEPT" and "REJECT" */
    T_ACCEPT, DROP, T_REJECT,
    MASQ, REDIRECT,
    OPENBRACE, CLOSEBRACE,
    OPENBRACKET, CLOSEBRACKET,
    SEMICOLON, STRING,
    INCLUDE,
    LOCALONLY, ROUTEDONLY,		/* for F_RTYPE */
    TEXT,				/* for F_LOG */
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
	struct {
	    enum filtertype direction;
	    char *iface;
	} ifinfo;
	enum filtertype target;
	char *logmsg;
	enum filtertype rtype;
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
filter_tctor new_filter_target, new_filter_rtype;
struct filter *new_filter_neg(struct filter *sub);
struct filter *new_filter_sibs(struct filter *list);
struct filter *new_filter_subgroup(char *name, struct filter *list);
typedef struct filter *filter_ctor(enum filtertype, const char*);
filter_ctor new_filter_device, new_filter_host, new_filter_ports, new_filter_icmp, new_filter_proto, new_filter_log;
struct filter *new_filter_oneway(void);

/* filter manipulations */
void filter_unroll(struct filter **f);
void filter_nogroup(struct filter *f);
void filter_noneg(struct filter **f);
void filter_apply_flags(struct filter *f, long flags);


/* from generated lexer and parer in filterlex.l */
int filter_fopen(const char *filename);
struct filter *filter_parse_list(void);

/* from gen.c */
#define	ESET(e,f)	(e->whats_set & (1 << F_ ##f))
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
    char *logmsg;
    int oneway;

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
typedef int filter_flush(enum filtertype policy);
filter_flush flush_iptables, flush_ipchains;

/* ("flags" arguments) */
#define	FF_NOSKEL	(1 << 0)	/* omit any "skeleton" rules */
#define	FF_LSTATE	(1 << 1)	/* lightweight state matching */
#define	FF_LOCAL	(1 << 2)	/* assume packets are local only */
#define	FF_ROUTE	(1 << 3)	/* assume packets are forwarded */
#define	FF_LOOKUP	(1 << 4)	/* translate host and service names into
					 * IP addresses and port numbers */
#define	FF_FLUSH	(1 << 5)	/* just flush the ruleset instead */

/* filtergen.c */
int oputs(const char *s);
int oprintf(const char *fmt, ...) _PRINTF_ATTR(1,2);

#endif /* _FK_FILTER_H */
