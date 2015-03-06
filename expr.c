/* See LICENSE file for copyright and license details. */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "utf.h"
#include "util.h"

/* token types for lexing/parsing
 * single character operators represent themselves */
enum {
	VAL = CHAR_MAX + 1, GE, LE, NE
};

typedef struct {
	char *s; /* iff s is NULL, Val is an integer */
	intmax_t n;
} Val;

static size_t intlen;

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

static char *
valstr(Val val, char *buf, size_t bufsiz)
{
	if (val.s)
		return val.s;
	snprintf(buf, bufsiz, "%"PRIdMAX, val.n);
	return buf;
}

static int
valcmp(Val a, Val b)
{
	char buf1[intlen], buf2[intlen];
	char *astr = valstr(a, buf1, sizeof(buf1));
	char *bstr = valstr(b, buf2, sizeof(buf2));

	if (!a.s && !b.s)
		return (a.n > b.n) - (a.n < b.n);
	return strcmp(astr, bstr);
}

/* match vstr against BRE vregx (treat both values as strings)
 * if there is at least one subexpression \(...\)
 * then return the text matched by it \1 (empty string for no match)
 * else return number of characters matched (0 for no match)
 */
static Val
match(Val vstr, Val vregx)
{
	regex_t re;
	regmatch_t matches[2];
	intmax_t d;
	char *s, *p, buf1[intlen], buf2[intlen];
	char *str = valstr(vstr, buf1, sizeof(buf1));
	char *regx = valstr(vregx, buf2, sizeof(buf2));;
	char anchreg[strlen(regx) + 2];

	/* expr(1p) "all patterns are anchored to the beginning of the string" */
	snprintf(anchreg, sizeof(anchreg), "^%s", regx);
	enregcomp(3, &re, anchreg, 0);

	if (regexec(&re, str, 2, matches, 0)) {
		regfree(&re);
		return (Val){ (re.re_nsub ? "" : NULL), 0 };
	}

	if (re.re_nsub) {
		regfree(&re);
		s = str + matches[1].rm_so;
		p = str + matches[1].rm_eo;

		*p = '\0';
		d = strtoimax(s, &p, 10);
		if (*s && !*p) /* string matched by subexpression is an integer */
			return (Val){ NULL, d };

		/* FIXME? string is never free()d, worth fixing?
		 * need to allocate as it could be in buf1 instead of vstr.s */
		return (Val){ enstrdup(3, s), 0 };
	}
	regfree(&re);
    str += matches[0].rm_so;
	return (Val){ NULL, utfnlen(str, matches[0].rm_eo - matches[0].rm_so) };
}

/* ops  points to a stack of operators, opp  points to one past the last op
 * vals points to a stack of values   , valp points to one past the last val
 * guaranteed that opp != ops
 * ops is unused here, but still included for parity with vals
 * pop operator, pop two values, apply operator, push result
 */
static void
doop(int *ops, int **opp, Val *vals, Val **valp)
{
	Val ret, a, b;
	int op;

	/* For an operation, we need a valid operator
	 * and two values on the stack */
	if ((*opp)[-1] == '(')
		enprintf(2, "syntax error: extra (\n");
	if (*valp - vals < 2)
		enprintf(2, "syntax error: missing expression or extra operator\n");

	a = (*valp)[-2];
	b = (*valp)[-1];
	op = (*opp)[-1];

	switch (op) {
	case '|':
		if      ( a.s && *a.s) ret = (Val){ a.s ,   0 };
		else if (!a.s &&  a.n) ret = (Val){ NULL, a.n };
		else if ( b.s && *b.s) ret = (Val){ b.s ,   0 };
		else                   ret = (Val){ NULL, b.n };
		break;
	case '&':
		if (((a.s && *a.s) || a.n) && ((b.s && *b.s) || b.n))
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

	case '+': enan(a); enan(b);             ret = (Val){ NULL, a.n + b.n }; break;
	case '-': enan(a); enan(b);             ret = (Val){ NULL, a.n - b.n }; break;
	case '*': enan(a); enan(b);             ret = (Val){ NULL, a.n * b.n }; break;
	case '/': enan(a); enan(b); ezero(b.n); ret = (Val){ NULL, a.n / b.n }; break;
	case '%': enan(a); enan(b); ezero(b.n); ret = (Val){ NULL, a.n % b.n }; break;

	case ':': ret = match(a, b); break;
	}

	(*valp)[-2] = ret;
	(*opp)--;
	(*valp)--;
}

/* retrn the type of the next token, s
 * if it is a value, place the value in v for use by parser
 */
static int
lex(char *s, Val *v)
{
	intmax_t d;
	char *p, *ops = "|&=><+-*/%():";

	/* clean integer */
	d = strtoimax(s, &p, 10);
	if (*s && !*p) {
		*v = (Val){ NULL, d };
		return VAL;
	}

	/* one-char operand */
	if (*s && !s[1] && strchr(ops, *s))
		return *s;

	/* two-char operand */
	if (!strcmp(s, ">=")) return GE;
	if (!strcmp(s, "<=")) return LE;
	if (!strcmp(s, "!=")) return NE;

	/* nothing matched, treat as string */
	*v = (Val){ s, 0 };
	return VAL;
}

/* using shunting-yard to convert from infix to rpn
 * https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 * instead of creating rpn output to evaluate later, evaluate it immediately as
 * it is created
 * vals is the value    stack, valp points to one past last value on the stack
 * ops  is the operator stack, opp  points to one past last op    on the stack
 */
static int
parse(char *expr[], int exprlen)
{
	Val vals[exprlen], *valp = vals, v;
	int ops[exprlen], *opp = ops;
	int i, type, lasttype = 0;
	char prec[] = { /* precedence of operators */
		['|'] = 1,
		['&'] = 2,
		['='] = 3, ['>'] = 3, [GE] = 3, ['<'] = 3, [LE] = 3, [NE] = 3,
		['+'] = 4, ['-'] = 4,
		['*'] = 5, ['/'] = 5, ['%'] = 5,
		[':'] = 6,
	};

	for (i = 0; i < exprlen; i++) {
		switch ((type = lex(expr[i], &v))) {
		case VAL:
			*valp++ = v;
			break;
		case '(':
			*opp++ = '(';
			break;
		case ')':
			if (lasttype == '(')
				enprintf(2, "syntax error: empty ( )\n");
			while (opp > ops && opp[-1] != '(')
				doop(ops, &opp, vals, &valp);
			if (opp == ops)
				enprintf(2, "syntax error: extra )\n");
			opp--;
			break;
		default: /* operator */
			if (prec[lasttype])
				enprintf(2, "syntax error: extra operator\n");
			while (opp > ops && prec[opp[-1]] >= prec[type])
				doop(ops, &opp, vals, &valp);
			*opp++ = type;
			break;
		}
		lasttype = type;
	}
	while (opp > ops)
		doop(ops, &opp, vals, &valp);

	if (valp == vals)
		enprintf(2, "syntax error: missing expression\n");
	if (--valp != vals)
		enprintf(2, "syntax error: extra expression\n");

	if (valp->s)
		printf("%s\n", valp->s);
	else
		printf("%"PRIdMAX"\n", valp->n);

	return (valp->s && *valp->s) || valp->n;
}

/* the only way to get usage() is if the user didn't supply -- and expression
 * begins with a -
 * expr(1p): "... the conforming application must employ the -- construct ...
 * if there is any chance the first operand might be a negative integer (or any
 * string with a leading minus"
 */
static void
usage(void)
{
	enprintf(3, "usage: %s [--] expression\n"
	            "note : the -- is mandatory if expression begins with a -\n", argv0);
}

int
main(int argc, char *argv[])
{
	intmax_t n = INTMAX_MIN;

	/* Get the maximum number of digits (+ sign) */
	for (intlen = (n < 0); n; n /= 10, ++intlen)
		;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	return !parse(argv, argc);
}
