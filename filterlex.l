%{
/*
 * lexer/parser for packet filtering rules
 *
 * $Id: filterlex.l,v 1.11 2002/08/20 17:29:08 matthew Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>

#include <string.h>

#include "filter.h"

#define	YY_NO_UNPUT

#define	MAXINCLUDES	16
struct filterparse_state {
	int incldepth;
	struct {
		YY_BUFFER_STATE bufferstat;
		int lineno;
	} incl[MAXINCLUDES];
} state = { 0, { { lineno: 1 } } };

#define	yylineno	state.incl[state.incldepth].lineno

%}

%%

include		{ return INCLUDE; }

#.*$		{ /* ignore comments */ }
[ \t]		{ /* ignore whitespace */ }
\n		{ ++yylineno; }

[;]		{ return SEMICOLON; }
[{]		{ return OPENBRACE; }
[}]		{ return CLOSEBRACE; }

[[]		{ return OPENBRACKET; }
[]]		{ return CLOSEBRACKET; }

[!]		{ return F_NEG; }

accept		{ return F_ACCEPT; }
drop		{ return F_DROP; }
reject		{ return F_REJECT; }

log		{ return F_LOG; }

masq		{ return F_MASQ; }
redirect	{ return F_REDIRECT; }
proxy		{ return F_REDIRECT; }

input		{ return F_INPUT; }
output		{ return F_OUTPUT; }

proto		{ return F_PROTO; }

source		{ return F_SOURCE; }
dest		{ return F_DEST; }
sport		{ return F_SPORT; }
dport		{ return F_DPORT; }

icmptype	{ return F_ICMPTYPE; }

local		{ return LOCALONLY; }
forward		{ return ROUTEDONLY; }

[-"_:[:alnum:]./+]+ { return STRING; }

%%

int filter_yylex(void)
{
	int tok = yylex();
	FILE *incl;

	switch(tok) {
	case INCLUDE:
		yylex();
		if(state.incldepth >= MAXINCLUDES+1)
			fprintf(stderr, "%d: too many nested includes\n", yylineno);
		/* XXX - make more effort to find file */
		else if(!(incl = fopen(yytext, "r")))
			fprintf(stderr, "%d: couldn't open include file \"%s\"\n", yylineno, yytext);
		else {
			state.incl[state.incldepth++].bufferstat = YY_CURRENT_BUFFER;
			state.incl[state.incldepth].lineno = 1;
			yy_switch_to_buffer(yy_create_buffer(yyin=incl, YY_BUF_SIZE));
		}
		tok = filter_yylex();
		break;

	case YYEOF:
		if(!state.incldepth) {
			yyterminate();
			return YYEOF;
		}
		state.incldepth--;
		yy_switch_to_buffer(state.incl[state.incldepth].bufferstat);
		/* skip semicolon after include clause */
		if((tok = filter_yylex()) == SEMICOLON)
			tok = filter_yylex();
		break;
	}
	return tok;
}


/*
 * Parser lives below here
 */

static struct filter *filter_parse_sibs(void)
{
	struct filter *f;
	enum filtertype t;
	f = filter_parse_list();
	t = filter_yylex();
	if(t != CLOSEBRACE) {
		fprintf(stderr, "%d: group not ended by a brace, but by \"%s\"\n",
				yylineno, yytext);
		yyless(0);
	}
	return new_filter_sibs(f);
}


static struct filter *filter_parse_subgroup(void)
{
	char *name;
	struct filter *f;
	enum filtertype t;

	filter_yylex();
	if(yytext[0] == '"') {
		if((name = strchr(yytext+1, '"')))
			*name = 0;
		name = strdup(yytext+1);
	} else {
		/* Allocate a filter name */
		static int ccount = 0;
		char nbuf[100];
		snprintf(nbuf, sizeof(nbuf)-1, "chain_%d", ccount++);
		nbuf[sizeof(nbuf)-1] = 0;
		name = strdup(nbuf);
		yyless(0);
	}
	f = filter_parse_list();
	t = filter_yylex();
	if(t != CLOSEBRACKET) {
		fprintf(stderr, "%d: group not ended by a bracket, but by \"%s\"\n",
				yylineno, yytext);
		yyless(0);
	}
	return new_filter_subgroup(name, f);
}


static struct filter *filter_parse_group(enum filtertype itok,
			filter_ctor *ctor)
{
	enum filtertype tok;
	struct filter *res = NULL, *end = NULL, *cur;

	if(!ctor) abort();

	tok = filter_yylex();
	if(tok != OPENBRACE)
		return ctor(itok, yytext);

	tok = filter_yylex();
	do {
		cur = ctor(itok, yytext);
		if(end)	end->next = cur;
		else	res = cur;
		end = cur;

		/* semicolons are optional here */
		if((tok = filter_yylex()) == SEMICOLON) tok = filter_yylex();
	} while(tok && (tok != CLOSEBRACE));
	end->next = NULL;
	if(res->next) res = new_filter_sibs(res);
	return res;
}

static struct filter *filter_parse_single(void)
{
	enum filtertype tok;

	tok = filter_yylex();
	switch(tok) {
	case F_NEG: return new_filter_neg(filter_parse_single());
	case F_INPUT: case F_OUTPUT:
		return filter_parse_group(tok, new_filter_device);
	case F_ACCEPT: case F_DROP: case F_REJECT:
	case F_MASQ: case F_REDIRECT:
		return new_filter_target(tok);
	case F_LOG:
		return new_filter_log(tok);
	case F_SOURCE: case F_DEST:
		return filter_parse_group(tok, new_filter_host);
	case F_SPORT: case F_DPORT:
		return filter_parse_group(tok, new_filter_ports);
	case F_ICMPTYPE:
		return filter_parse_group(tok, new_filter_icmp);
	case F_PROTO:
		return filter_parse_group(tok, new_filter_proto);
	case LOCALONLY: case ROUTEDONLY:
		return new_filter_rtype(tok);
	case OPENBRACE:
		return filter_parse_sibs();
	case OPENBRACKET:
		return filter_parse_subgroup();
	case CLOSEBRACE: case CLOSEBRACKET:
	case SEMICOLON: yyless(0);
	case YYEOF:	break;
	default:
		fprintf(stderr, "%d: unknown token \"%s\"\n", yylineno, yytext);
	}
	return NULL;
}


static struct filter *filter_parse_chain(void)
{
	struct filter *res = NULL, *end = NULL, *new;
	enum filtertype tok;

	do {
		new = filter_parse_single();
		if(end)	end->child = new;
		else	res = new;
		end = new;

		tok = filter_yylex();
		if(!tok || (tok == SEMICOLON)) break;
		yyless(0);
	} while(new);
	if(end) end->child = NULL;
	return res;
}

struct filter *filter_parse_list(void)
{
	struct filter *res = NULL, *end = NULL, *new;
	while((new = filter_parse_chain())) {
		if(end)	end->next = new;
		else	res = new;
		end = new;
	}
	if(end) end->next = NULL;
	return res;
}
