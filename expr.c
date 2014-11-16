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
static int yylex(void);
static int yyparse(int);

static char **args;
static size_t intlen;
static Val yylval;

static void
ezero(intmax_t n)
{
	if(n == 0)
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
		else if (!a.s &&  a.n)
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
		ret = emalloc(len); /* TODO: free ret */
		d = strtoimax(ret, &p, 10);
		strlcpy(ret, str + matches[1].rm_so, len);

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
yylex(void)
{
	intmax_t d;
	char *q, *p, *ops = "|&=><+-*/%():";

	if (!(p = *args++))
		return 0;

	d = strtoimax(p, &q, 10);
	if (*p && !*q) {
		yylval = (Val){ NULL, d };
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

	yylval = (Val){ p, 0 };
	return VAL;
}

static int
yyparse(int argc)
{
	Val vals[argc], *vtop = vals;
	int ops [argc], *otop = ops;
	int type, last = 0;
	char prec[] = {
		['|'] = 1,
		['&'] = 2,
		['='] = 3, ['>'] = 3, [GE] = 3, ['<'] = 3, [LE] = 3, [NE] = 3,
		['+'] = 4, ['-'] = 4,
		['*'] = 5, ['/'] = 5, ['%'] = 5,
		[':'] = 6,
	};

	while((type = yylex())) {
		switch (type) {
		case VAL: *vtop++ = yylval; break;
		case '(': *otop++ = '('   ; break;
		case ')':
			if (last == '(')
				enprintf(2, "syntax error: empty ( )\n");
			while(otop > ops && otop[-1] != '(')
				doop(ops, &otop, vals, &vtop);
			if (otop == ops)
				enprintf(2, "syntax error: extra )\n");
			otop--;
			break;
		default :
			if (prec[last])
				enprintf(2, "syntax error: extra operator\n");
			while (otop > ops && prec[otop[-1]] >= prec[type])
				doop(ops, &otop, vals, &vtop);
			*otop++ = type;
			break;
		}
		last = type;
	}
	while(otop > ops)
		doop(ops, &otop, vals, &vtop);

	if (vtop == vals)
		enprintf(2, "syntax error: missing expression\n");
	if (vtop - vals > 1)
		enprintf(2, "syntax error: extra expression\n");

	vtop--;
	if (vtop->s)
		printf("%s\n", vtop->s);
	else
		printf("%"PRIdMAX"\n", vtop->n);

	return (vtop->s && *vtop->s) || vtop->n;
}

int
main(int argc, char **argv)
{
	if (!(intlen = snprintf(NULL, 0, "%"PRIdMAX, INTMAX_MIN) + 1))
		enprintf(3, "failed to get max digits\n");

	args = argv + 1;
	if (*args && !strcmp("--", *args))
		++args;

	return !yyparse(argc);
}
