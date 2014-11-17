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

static void enan(Val);
static void ezero(intmax_t);
static void doop(int*, int**, Val*, Val**);
static Val match(Val, Val);
static int valcmp(Val, Val);
static char *valstr(Val, char *, size_t);
static int lex(char *);
static int parse(char **, int);

static size_t intlen;
static Val lastval;

static void
enan(Val v)
{
	if (v.s)
		enprintf(2, "syntax error: expected integer got `%s'\n", v.s);
}

static void
ezero(intmax_t n)
{
	if (n == 0)
		enprintf(2, "division by zero\n");
}

static void
doop(int *op, int **opp, Val *val, Val **valp)
{
	Val ret, a, b;
	int o;

	/* For an operation, we need a valid operator
	 * and two values on the stack */
	if ((*opp)[-1] == '(')
		enprintf(2, "syntax error: extra (\n");
	if (*valp - val < 2)
		enprintf(2, "syntax error: missing expression or extra operator\n");

	a = (*valp)[-2];
	b = (*valp)[-1];
	o = (*opp)[-1];

	switch (o) {
	case '|':
		if (a.s && *a.s) {
			ret = (Val){ a.s, 0 };
		} else if (!a.s && a.n) {
			ret = (Val){ NULL, a.n };
		} else if (b.s && *b.s) {
			ret = (Val){ b.s, 0 };
		} else {
			ret = (Val){ NULL, b.n };
		}
		break;
	case '&':
		if (((a.s && *a.s) || a.n) &&
		    ((b.s && *b.s) || b.n)) {
			ret = a;
		} else {
			ret = (Val){ NULL, 0 };
		}
		break;
	case '=': ret = (Val){ NULL, valcmp(a, b) == 0 }; break;
	case '>': ret = (Val){ NULL, valcmp(a, b) >  0 }; break;
	case GE : ret = (Val){ NULL, valcmp(a, b) >= 0 }; break;
	case '<': ret = (Val){ NULL, valcmp(a, b) <  0 }; break;
	case LE : ret = (Val){ NULL, valcmp(a, b) <= 0 }; break;
	case NE : ret = (Val){ NULL, valcmp(a, b) != 0 }; break;

	case '+': enan(a); enan(b); ret = (Val){ NULL, a.n + b.n }; break;
	case '-': enan(a); enan(b); ret = (Val){ NULL, a.n - b.n }; break;
	case '*': enan(a); enan(b); ret = (Val){ NULL, a.n * b.n }; break;
	case '/': enan(a); enan(b); ezero(b.n); ret = (Val){ NULL, a.n / b.n }; break;
	case '%': enan(a); enan(b); ezero(b.n); ret = (Val){ NULL, a.n % b.n }; break;

	case ':': ret = match(a, b); break;
	}

	(*valp)[-2] = ret;
	(*opp)--;
	(*valp)--;
}

static Val
match(Val vstr, Val vregx)
{
	intmax_t d;
	char *anchreg, *ret, *p;
	char buf1[intlen], buf2[intlen], *str, *regx;
	regoff_t len;
	regex_t re;
	regmatch_t matches[2];

	str = valstr(vstr, buf1, sizeof(buf1));
	regx = valstr(vregx, buf2, sizeof(buf2));

	anchreg = malloc(strlen(regx) + 2);
	if (!anchreg)
		enprintf(3, "malloc:");
	snprintf(anchreg, strlen(regx) + 2, "^%s", regx);

	enregcomp(3, &re, anchreg, 0);
	free(anchreg);

	if (regexec(&re, str, 2, matches, 0)) {
		regfree(&re);
		return (Val){ (re.re_nsub ? "" : NULL), 0 };
	}

	if (re.re_nsub) {
		regfree(&re);
		len = matches[1].rm_eo - matches[1].rm_so + 1;
		ret = malloc(len);
		if (!ret)
			enprintf(3, "malloc:");
		strlcpy(ret, str + matches[1].rm_so, len);
		d = strtoimax(ret, &p, 10);
		if (*ret && !*p) {
			free(ret);
			return (Val){ NULL, d };
		}
		return (Val){ ret, 0 };
	}
	regfree(&re);
	return (Val){ NULL, matches[0].rm_eo - matches[0].rm_so };
}



static int
valcmp(Val a, Val b)
{
	char buf1[intlen], buf2[intlen], *astr, *bstr;

	astr = valstr(a, buf1, sizeof(buf1));
	bstr = valstr(b, buf2, sizeof(buf2));

	return strcmp(astr, bstr);
}

static char *
valstr(Val val, char *buf, size_t bufsiz)
{
	if (val.s)
		return val.s;
	snprintf(buf, bufsiz, "%"PRIdMAX, val.n);
	return buf;
}

static int
lex(char *p)
{
	intmax_t d;
	char *q, *ops = "|&=><+-*/%():";

	/* clean integer */
	d = strtoimax(p, &q, 10);
	if (*p && !*q) {
		lastval = (Val){ NULL, d };
		return VAL;
	}

	/* one-char operand */
	if (*p && !*(p+1) && strchr(ops, *p))
		return *p;

	/* two-char operand */
	if (*p && *(p+1) == '=' && !*(p+2)) {
		switch (*p) {
		case '>':
			return GE;
		case '<':
			return LE;
		case '!':
			return NE;
		}
	}

	/* nothing matched, treat as string */
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
