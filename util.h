/*
 * Utils for filtergen output
 *
 * $Id: util.h,v 1.1 2001/10/03 19:32:57 matthew Exp $
 */
#ifndef	_FILTER_UTIL_H
#define	_FILTER_UTIL_H	1

#define	APP(l,s)	(l = strapp(l,s))
#define	APP2(l,s1,s2)	(l = strapp2(l,s1,s2))
#define	APPS(l,s)	APP2(l, " ", s)
#define	APPS2(l,s1,s2)	(APPS(l,s1), APP(l,s2))
#define	APPSS2(l,s1,s2)	(APPS(l,s1), APPS(l,s2))

#define	NEG(t)		(ent->whats_negated & (1<<F_ ## t))
#define	NEGA(l,t)	(NEG(t) ? APPS(l, "!") : l)

#endif /* _FILTER_UTIL_H */
