/* See LICENSE file for copyright and license details. */
#include <inttypes.h>
#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

enum {
	VAL = CHAR_MAX + 1, GE, LE, NE
};

typedef struct {
	char *s;
	intmax_t n;
} Val;

static void doop(int*, int**, Val*, Val**);
static Val match(Val, Val);
static void num(Val);
static int valcmp(Val, Val);
static char *valstr(Val, char*, size_t);
static int lex(char *);
static int parse(char **, int);

static size_t intlen;
static Val lastval;

static void
ezero(intmax_t n)
{
	if (n == 0)
		enprintf(2, "division by zero\n");
}

/* otop points to one past last op
 * vtop points to one past last val
 * guaranteed otop != ops
 * pop two vals, pop op, apply op, push val
 */
static void
doop(int *ops, int **otop, Val *vals, Val **vtop)
{
	Val ret, a, b;
	int op;

	if ((*otop)[-1] == '(')
		enprintf(2, "syntax error: extra (\n");
	if (*vtop - vals < 2)
		enprintf(2, "syntax error: missing expression or extra operator\n");

	a = (*vtop)[-2];
	b = (*vtop)[-1];
	op = (*otop)[-1];

	switch (op) {
	case '|':
		if (a.s && *a.s)
			ret = (Val){ a.s, 0 };
		else if (!a.s && a.n)
			ret = (Val){ NULL, a.n };
		else if (b.s && *b.s)
			ret = (Val){ b.s, 0 };
		else
			ret = (Val){ NULL, b.n };
		break;
	case '&':
		if (((a.s && *a.s) || a.n) &&
		   ((b.s && *b.s) || b.n))
			ret = a;
		else
			ret = (Val){ NULL, 0 };
		break;
	case '=': ret = (Val){ NULL, valcmp(a, b) == 0 }; break;
	case '>': ret = (Val){ NULL, valcmp(a, b) >  0 }; break;
	case GE : ret = (Val){ NULL, valcmp(a, b) >= 0 }; break;
	case '<': ret = (Val){ NULL, valcmp(a, b) <  0 }; break;
	case LE : ret = (Val){ NULL, valcmp(a, b) <= 0 }; break;
	case NE : ret = (Val){ NULL, valcmp(a, b) != 0 }; break;

	case '+': num(a); num(b); ret = (Val){ NULL, a.n + b.n }; break;
	case '-': num(a); num(b); ret = (Val){ NULL, a.n - b.n }; break;
	case '*': num(a); num(b); ret = (Val){ NULL, a.n * b.n }; break;
	case '/': num(a); num(b); ezero(b.n); ret = (Val){ NULL, a.n / b.n }; break;
	case '%': num(a); num(b); ezero(b.n); ret = (Val){ NULL, a.n % b.n }; break;

	case ':': ret = match(a, b); break;
	}

	(*vtop)[-2] = ret;
	(*otop)--;
	(*vtop)--;
}

static Val
match(Val vstr, Val vregx)
{
	intmax_t d;
	char    *ret, *p;
	regoff_t len;
	char b1[intlen], *str  = valstr(vstr, b1, sizeof(b1));
	char b2[intlen], *regx = valstr(vregx, b2, sizeof(b2));

	regex_t    re;
	regmatch_t matches[2];
	char       anchreg[strlen(regx) + 2];

	snprintf(anchreg, sizeof(anchreg), "^%s", regx);
	enregcomp(3, &re, anchreg, 0);

	if (regexec(&re, str, 2, matches, 0))
		return (Val){ (re.re_nsub ? "" : NULL), 0 };

	if (re.re_nsub) {
		len = matches[1].rm_eo - matches[1].rm_so + 1;
		ret = malloc(len); /* TODO: free ret */
		if (!ret)
			enprintf(3, "malloc:");
		strlcpy(ret, str + matches[1].rm_so, len);
		d = strtoimax(ret, &p, 10);
		if (*ret && !*p)
			return (Val){ NULL, d };
		return (Val){ ret, 0 };
	}
	return (Val){ NULL, matches[0].rm_eo - matches[0].rm_so };
}

static void
num(Val v)
{
	if (v.s)
		enprintf(2, "syntax error: expected integer got `%s'\n", v.s);
}

static int
valcmp(Val a, Val b)
{
	char b1[intlen], *p = valstr(a, b1, sizeof(b1));
	char b2[intlen], *q = valstr(b, b2, sizeof(b2));

	if (!a.s && !b.s)
		return (a.n > b.n) - (a.n < b.n);
	return strcmp(p, q);
}

static char *
valstr(Val val, char *buf, size_t bufsiz)
{
	char *p = val.s;
	if (!p) {
		snprintf(buf, bufsiz, "%"PRIdMAX, val.n);
		p = buf;
	}
	return p;
}

static int
lex(char *p)
{
	intmax_t d;
	char *q, *ops = "|&=><+-*/%():";

	d = strtoimax(p, &q, 10);
	if (*p && !*q) {
		lastval = (Val){ NULL, d };
		return VAL;
	}

	if (*p && !p[1] && strchr(ops, *p))
		return *p;

	if (strcmp(p, ">=") == 0)
		return GE;
	if (strcmp(p, "<=") == 0)
		return LE;
	if (strcmp(p, "!=") == 0)
		return NE;

	lastval = (Val){ p, 0 };
	return VAL;
}

static int
parse(char **expr, int exprlen)
{
	Val val[exprlen], *valp = val;
	int op[exprlen], *opp = op;
	int i, type, lasttype = 0;
	char prec[] = {
		[ 0 ] = 0, [VAL] = 0,
		['|'] = 1,
		['&'] = 2,
		['='] = 3, ['>'] = 3, [GE] = 3, ['<'] = 3, [LE] = 3, [NE] = 3,
		['+'] = 4, ['-'] = 4,
		['*'] = 5, ['/'] = 5, ['%'] = 5,
		[':'] = 6,
	};

	for (i = 0; i < exprlen; i++) {
		type = lex(expr[i]);

		switch (type) {
		case VAL:
			*valp++ = lastval;
			break;
		case '(':
			*opp++ = '(';
			break;
		case ')':
			if (lasttype == '(')
				enprintf(2, "syntax error: empty ( )\n");
			while (opp > op && opp[-1] != '(')
				doop(op, &opp, val, &valp);
			if (opp == op)
				enprintf(2, "syntax error: extra )\n");
			opp--;
			break;
		default :
			if (prec[lasttype])
				enprintf(2, "syntax error: extra operator\n");
			while (opp > op && prec[opp[-1]] >= prec[type])
				doop(op, &opp, val, &valp);
			*opp++ = type;
			break;
		}
		lasttype = type;
	}
	while (opp > op)
		doop(op, &opp, val, &valp);

	if (valp == val)
		enprintf(2, "syntax error: missing expression\n");
	if (valp - val > 1)
		enprintf(2, "syntax error: extra expression\n");

	valp--;
	if (valp->s)
		printf("%s\n", valp->s);
	else
		printf("%"PRIdMAX"\n", valp->n);

	return (valp->s && *valp->s) || valp->n;
}

static void
usage(void)
{
	eprintf("usage: %s EXPRESSION\n", argv0);
}

int
main(int argc, char *argv[])
{
	intmax_t n = INTMAX_MIN;

	/* Get the maximum number of digits (+ sign) */
	for (intlen = (n < 0); n; n /= 10, ++intlen);

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	return !parse(argv, argc);
}
