/* $Id: filter.c,v 1.3 2001/10/06 18:25:16 matthew Exp $ */

#include <stdlib.h>
#include <string.h>

#include "filter.h"


struct filter *__new_filter(enum filtertype type)
{
	struct filter *f;
	if ((f = calloc(1, sizeof(*f)))) {
		f->type = type;
	}
	return f;
}


struct filter *new_filter_target(enum filtertype target)
{
	struct filter *f;
	if ((f = __new_filter(F_TARGET))) {
		f->u.target = target;
	}
	return f;
}


struct filter *new_filter_log(enum filtertype ltype)
{
	struct filter *f;
	if ((f = __new_filter(F_LOG))) {
		f->u.log = ltype;
	}
	return f;
}


struct filter *new_filter_neg(struct filter *sub)
{
	struct filter *f;
	if ((f = __new_filter(F_NEG))) {
		f->u.neg = sub;
	}
	return f;
}

struct filter *new_filter_sibs(struct filter *list)
{
	struct filter *f;
//	if(!list || !list->next) return list;
	if ((f = __new_filter(F_SIBLIST))) {
		f->u.sib = list;
	}
	return f;
}



struct filter *new_filter_proto(enum filtertype proto)
{
	struct filter *f;
	if ((f = __new_filter(F_PROTO))) {
		f->u.proto = proto;
	}
	return f;
}

struct filter *new_filter_device(enum filtertype type, const char *iface)
{
	struct filter *f;
	if ((f = __new_filter(type))) {
		f->u.iface = strdup(iface);
	}
	return f;
}


struct filter *new_filter_host(enum filtertype type, const char *matchstr)
{
	struct filter *f;
	if ((f = __new_filter(type))) {
		f->u.addrs = strdup(matchstr);
	}
	return f;
}


struct filter *new_filter_ports(enum filtertype type, const char *matchstr)
{
	struct filter *f;
	if ((f = __new_filter(type))) {
		f->u.ports = strdup(matchstr);
	}
	return f;
}


/*
 * These functions DAGify the parsed filter structure.
 * This means that we can walk all paths down the DAG
 * merely by following ->child and ->next.
 */

static void filter_append(struct filter *f, struct filter *x)
{
	if(!f) abort();
	while((f->type != F_SIBLIST) && f->child)
		f = f->child;

	if(f->type == F_SIBLIST) {
		if(f->child) abort();
		for(f=f->u.sib;f;f=f->next)
			filter_append(f,x);
	} else
		f->child = x;
}

/*
 * The easy bit of a cross-product.  Basically we ensure that no
 * filter node has more than one path out.
 * 1. We push sibling->child down to the end of the component
 *    sub-lists, and
 * 2. Ensure that negated entries have only a single component
 *    hanging off them.
 */
void __filter_unroll(struct filter *f)
{
	struct filter *c, *s;

	if(!f) return;

	/* depth first */
	__filter_unroll(c = f->child);

	/* check this node */
	switch(f->type) {
	case F_SIBLIST:
		for(s = f->u.sib; s; s = s->next) {
			__filter_unroll(s);
			filter_append(s, c);
		}
		f->child = NULL;
		break;
	case F_NEG: abort();
	default: break;
	}

	/* lastly, go sideways */
	__filter_unroll(f->next);
}


void __filter_neg_expand(struct filter **f, int neg)
{
	if(!*f) return;
	__filter_neg_expand(&(*f)->child, neg);
	__filter_neg_expand(&(*f)->next, neg);

	switch((*f)->type) {
	case F_SIBLIST:	__filter_neg_expand(&(*f)->u.sib, neg); break;
	case F_NEG: {
		struct filter *c = (*f)->child;
		*f = (*f)->u.neg;
		neg = !neg;
		__filter_neg_expand(f, neg);
		if(c) filter_append(*f, c);
		break;
	}
	default: break;
	}
	(*f)->negate = neg;
}


/* Move targets to end of each list */
void __filter_targets_to_end(struct filter **f)
{
	if(!*f) return;
	if(((*f)->type == F_TARGET) && (*f)->child) {
		struct filter *c = (*f)->child;

		/* Unlink this one */
		(*f)->child = NULL;

		/* Append ourselves */
		filter_append(c, (*f));

		/* Tell them upstairs */
		*f = c;
	} else {
		__filter_targets_to_end(&(*f)->child);
		__filter_targets_to_end(&(*f)->next);
	}
}

void filter_unroll(struct filter **f)
{
	__filter_neg_expand(f, 0);
	__filter_targets_to_end(f);
	__filter_unroll(*f);
}


/* Remove negations by reordering tree:
 *	NEG(ent)->child,next
 * becomes
 *	ent->next,child,next
 */
void filter_noneg(struct filter **f)
{
	
}
