#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "arg.h"
#include "util.h"

#define NDIGITS 10
#define REGSIZ 16
#define HASHSIZ 128

enum {
	NVAL,
	STR,
	NUM,
};

enum {
	NE,
	LE,
	GE,
};

typedef struct num Num;
typedef struct val Val;

struct num {
	int begin;
	int scale;
	int room;
	signed char *buf, *wp, *rp;
};

struct digit {
	int val;
	struct digit *next;
};

struct val {
	int type;
	union {
		Num *n;
		char *s;
	} u;

	struct val *next;
};

struct ary {
	int n;
	Val *buf;
	struct ary *next;
};

struct reg {
	char *name;
	Val val;
	struct ary ary;
	struct reg *next;
};

struct input {
	FILE *fp;
	char *buf;
	size_t n;
	char *s;
	struct input *next;
};

static Val *stack;
static jmp_buf env;
static struct input *input;
static struct reg *htbl[HASHSIZ];

static signed char onestr[] = {1, 0};
static Num zero, one = {.buf = onestr, .wp = onestr + 1};
static char digits[] = "0123456789ABCDEF.";

static int scale, ibase = 10, obase = 10;
static int iflag;
static int col;

/*
 * This dc implementation follows the decription of dc found in the paper
 * DC - An Interactive Desk Calculator, by Robert Morris and Lorinda Cherry
 */

static void
dumpnum(char *msg, Num *num)
{
	signed char *p;

	printf("Num (%s)", msg ? msg : "");

	if (!num) {
		puts("NULL\n");
		return;
	}
	printf("\n"
	       "\tscale=%d\n"
	       "\troom=%d\n"
	       "\tdigits=[\n",
	       num->scale,
	       num->room);
	for (p = num->buf; p < num->wp; ++p)
		printf("\t\t%02d\n", *p);
	printf("\t]\n");
}

/*
 * error is not called from the implementation of the
 * arithmetic functions because that can drive to memory
 * leaks very easily.
 */
static void
error(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	xvprintf(fmt, va);
	putc('\n', stderr);
	va_end(va);

	longjmp(env, 1);
}

static void
freenum(Num *num)
{
	if (!num)
		return;
	free(num->buf);
	free(num);
}

static Num *
moreroom(Num *num, int more)
{
	int ro, wo, room;
	signed char *p;

	ro = num->rp - num->buf;
	wo = num->wp - num->buf;
	room = num->room;

	if (room > INT_MAX - more)
		eprintf("out of memory\n");

	room = room + more;
	if (room < NDIGITS)
		room = NDIGITS;
	p = erealloc(num->buf, room);
	num->buf = p;
	num->rp = p + ro;
	num->wp = p + wo;
	num->room = room;

	return num;
}

static Num *
grow(Num *num)
{
	return moreroom(num, NDIGITS);
}

static Num *
expand(Num *num, int min)
{
	if (min < num->room)
		return num;
	return moreroom(num, min - num->room);
}

static Num *
new(int room)
{
	Num *num = emalloc(sizeof(*num));

	num->rp = num->wp = num->buf = NULL;
	num->begin = num->room = num->scale = 0;

	return moreroom(num, room);
}

static Num *
zeronum(int ndigits)
{
	Num *num = new(ndigits);

	num->wp = num->buf + ndigits;
	memset(num->buf, 0, ndigits);

	return num;
}

static Num *
wrdigit(Num *num, int d)
{
	if (num->wp == &num->buf[num->room])
		grow(num);
	*num->wp++ = d;

	return num;
}

static int
rddigit(Num *num)
{
	if (num->rp == num->wp)
		return -1;
	return *num->rp++;
}

static int
peek(Num *num)
{
	if (num->rp == num->wp)
		return -1;
	return *num->rp;
}

static Num *
poke(Num *num, unsigned d)
{
	if (num->rp == &num->buf[num->room])
		grow(num);
	if (num->rp == num->wp)
		num->wp++;
	*num->rp = d;

	return num;
}

static int
begin(Num *num)
{
	return num->begin != 0;
}

static int
first(Num *num)
{
	num->begin = 0;
	num->rp = num->buf;
	return num->rp != num->wp;
}

static int
last(Num *num)
{
	if (num->wp != num->buf) {
		num->begin = 0;
		num->rp = num->wp - 1;
		return 1;
	}
	num->begin = 1;
	return 0;
}

static int
prev(Num *num)
{
	if (num->rp > num->buf) {
		--num->rp;
		return 1;
	}
	num->begin = 1;
	return 0;
}

static int
next(Num *num)
{
	if (num->rp != num->wp + 1) {
		++num->rp;
		return 1;
	}
	return 0;
}

static void
numtrunc(Num *num)
{
	num->wp = num->rp;
	if (num->rp != num->buf)
		num->rp--;
}

static int
more(Num *num)
{
	return (num->rp != num->wp);
}

static int
length(Num *num)
{
	return num->wp - num->buf;
}

static int
tell(Num *num)
{
	return num->rp - num->buf;
}

static void
seek(Num *num, int pos)
{
	num->rp = num->buf + pos;
}

static void
rshift(Num *num, int n)
{
	int diff;

	diff = length(num) - n;
	if (diff < 0) {
		first(num);
		numtrunc(num);
		return;
	}

	memmove(num->buf, num->buf + n, diff);
	num->rp = num->buf + diff;
	numtrunc(num);
}

static void
lshift(Num *num, int n)
{
	int len;

	len = length(num);
	expand(num, len + n);
	memmove(num->buf + n, num->buf, len);
	memset(num->buf, 0, n);
	num->wp += n;
}

static Num *
chsign(Num *num)
{
	int val, d, carry;

	carry = 0;
	for (first(num); more(num); next(num)) {
		d = peek(num);
		val = 100 - d - carry;

		carry = 1;
		if (val >= 100) {
			val -= 100;
			carry = 0;
		}
		poke(num, val);
	}

	prev(num);
	if (carry != 0) {
		if (peek(num) == 99)
			poke(num, -1);
		else
			wrdigit(num, -1);
	} else {
		if (peek(num) == 0)
			numtrunc(num);
	}

	return num;
}

static Num *
copy(Num *num)
{
	Num *p;
	int len = length(num);

	p = new(len);
	memcpy(p->buf, num->buf, len);
	p->wp = p->buf + len;
	p->rp = p->buf;
	p->scale = num->scale;

	return p;
}

static int
negative(Num *num)
{
	return last(num) && peek(num) == -1;
}

static Num *
norm(Num *n)
{
	/* trailing 0 */
	for (last(n); peek(n) == 0; numtrunc(n))
		;

	if (negative(n)) {
		for (prev(n); peek(n) == 99; prev(n)) {
			poke(n, -1);
			next(n);
			numtrunc(n);
		}
	}

	/* adjust scale for 0 case*/
	if (length(n) == 0)
		n->scale = 0;
	return n;
}

static Num *
mulnto(Num *src, Num *dst, int n)
{
	div_t dd;
	int d, carry;

	first(dst);
	numtrunc(dst);

	carry = 0;
	for (first(src); more(src); next(src)) {
		d = peek(src) * n + carry;
		dd = div(d, 100);
		carry = dd.quot;
		wrdigit(dst, dd.rem);
	}

	if (carry)
		wrdigit(dst, carry);
	return dst;
}

static Num *
muln(Num *num, int n)
{
	div_t dd;
	int d, carry;

	carry = 0;
	for (first(num); more(num); next(num)) {
		d = peek(num) * n + carry;
		dd = div(d, 100);
		carry = dd.quot;
		poke(num, dd.rem);
	}

	if (carry)
		wrdigit(num, carry);
	return num;
}

static int
divn(Num *num, int n)
{
	div_t dd;
	int val, carry;

	carry = 0;
	for (last(num); !begin(num); prev(num)) {
		val = carry * 100 + peek(num);
		dd = div(val, n);
		poke(num, dd.quot);
		carry = dd.rem;
	}
	norm(num);

	return carry;
}

static void
div10(Num *num, int n)
{
	div_t dd = div(n, 2);

	if (dd.rem == 1)
		divn(num, 10);

	rshift(num, dd.quot);
}

static void
mul10(Num *num, int n)
{
	div_t dd = div(n, 2);

	if (dd.rem == 1)
		muln(num, 10);

	lshift(num, dd.quot);
}

static void
align(Num *a, Num *b)
{
	int d;
	Num *max, *min;

	d = a->scale - b->scale;
	if (d == 0) {
		return;
	} else if (d > 0) {
		min = b;
		max = a;
	} else {
		min = a;
		max = b;
	}

	d = abs(d);
	mul10(min, d);
	min->scale += d;

	assert(min->scale == max->scale);
}

static Num *
addn(Num *num, int val)
{
	int d, carry = val;

	for (first(num); carry; next(num)) {
		d = more(num) ? peek(num) : 0;
		d += carry;
		carry = 0;

		if (d >= 100) {
			carry = 1;
			d -= 100;
		}
		poke(num, d);
	}

	return num;
}

static Num *
reverse(Num *num)
{
	int d;
	signed char *p, *q;

	for (p = num->buf, q = num->wp - 1; p < q; ++p, --q) {
		d = *p;
		*p = *q;
		*q = d;
	}

	return num;
}

static Num *
addnum(Num *a, Num *b)
{
	Num *c;
	int len, alen, blen, carry, da, db, sum;

	align(a, b);
	alen = length(a);
	blen = length(b);
	len = (alen > blen) ? alen : blen;
	c = new(len);

	first(a);
	first(b);
	carry = 0;
	while (len-- > 0) {
		da = (more(a)) ? rddigit(a) : 0;
		db = (more(b)) ? rddigit(b) : 0;

		sum = da + db + carry;
		if (sum >= 100) {
			carry = 1;
			sum -= 100;
		} else if (sum < 0) {
			carry = -1;
			sum += 100;
		} else {
			carry = 0;
		}

		wrdigit(c, sum);
	}

	if (carry)
		wrdigit(c, carry);
	c->scale = a->scale;

	return norm(c);
}

static Num *
subnum(Num *a, Num *b)
{
	Num *tmp, *sum;

	tmp = chsign(copy(b));
	sum = addnum(a, tmp);
	freenum(tmp);

	return sum;
}

static Num *
mulnum(Num *a, Num *b)
{
	Num shadow, *c, *ca, *cb;
	int pos, prod, carry, dc, db, da, sc;
	int asign = negative(a), bsign = negative(b);

	c = zeronum(length(a) + length(b) + 1);
	c->scale = a->scale + b->scale;
	sc = (a->scale > b->scale) ? a->scale : b->scale;

	ca = a;
	if (asign)
		ca = chsign(copy(ca));
	cb = b;
	if (bsign)
		cb = chsign(copy(cb));

	/* avoid aliasing problems when called from expnum*/
	if (ca == cb) {
		shadow = *cb;
		b = cb = &shadow;
	}

	for (first(cb); more(cb); next(cb)) {
		div_t d;

		carry = 0;
		db = peek(cb);

		pos = tell(c);
		for (first(ca); more(ca); next(ca)) {
			da = peek(ca);
			dc = peek(c);
			prod = da * db + dc + carry;
			d = div(prod, 100);
			carry = d.quot;
			poke(c, d.rem);
			next(c);
		}

		for ( ; carry > 0; carry = d.quot) {
			dc = peek(c) + carry;
			d = div(dc, 100);
			poke(c, d.rem);
			next(c);
		}
		seek(c, pos + 1);
	}
	norm(c);

	if (sc < scale)
		sc = scale;
	sc = c->scale - sc;
	if (sc > 0) {
		div10(c, sc);
		c->scale -= sc;
	}

	if (ca != a)
		freenum(ca);
	if (cb != b)
		freenum(cb);

	if (asign ^ bsign)
		chsign(c);
	return c;
}

/*
 * The divmod function is implemented following the algorithm
 * from the plan9 version that is not exactly like the one described
 * in the paper. A lot of magic here.
 */
static Num *
divmod(Num *odivd, Num *odivr, Num **remp)
{
	Num *acc, *divd, *divr, *res;
	int divsign, remsign;
	int under, magic, ndig, diff;
	int d, q, carry, divcarry;
	long dr, dd, cc;

	divr = odivr;
	acc = copy(&zero);
	divd = copy(odivd);
	res = zeronum(length(odivd));

	under = divcarry = divsign = remsign = 0;

	if (length(divr) == 0) {
		weprintf("divide by 0\n");
		goto ret;
	}

	divsign = negative(divd);
	if (divsign)
		chsign(divd);

	remsign = negative(divr);
	if (remsign)
		divr = chsign(copy(divr));

	diff = length(divd) - length(divr);

	seek(res, diff + 1);
	last(divd);
	last(divr);

	wrdigit(divd, 0);

	dr = peek(divr);
	magic = dr < 10;
	dr = dr * 100 + (prev(divr) ? peek(divr) : 0);
	if (magic) {
		dr = dr * 100 + (prev(divr) ? peek(divr) : 0);
		dr *= 2;
		dr /= 25;
	}

	for (ndig = 0; diff >= 0; ++ndig) {
		last(divd);
		dd = peek(divd);
		dd = dd * 100 + (prev(divd) ? peek(divd) : 0);
		dd = dd * 100 + (prev(divd) ? peek(divd) : 0);
		cc = dr;

		if (diff == 0)
			dd++;
		else
			cc++;

		if (magic)
			dd *= 8;

		q = dd / cc;
		under = 0;
		if (q > 0 && dd % cc < 8 && magic) {
			q--;
			under = 1;
		}

		mulnto(divr, acc, q);

		/* Subtract acc from dividend at offset position */
		first(acc);
		carry = 0;
		for (seek(divd, diff); more(divd); next(divd)) {
			d = peek(divd);
			d = d - (more(acc) ? rddigit(acc) : 0) - carry;
			carry = 0;
			if (d < 0) {
				d += 100;
				carry = 1;
			}
			poke(divd, d);
		}
		divcarry = carry;

		/* Store quotient digit */
		prev(res);
		poke(res, q);

		/* Handle borrow propagation */
		last(divd);
		d = peek(divd);
		if ((d != 0) && (diff != 0)) {
			prev(divd);
			d = peek(divd) + 100;
			poke(divd, d);
		}

		/* Shorten dividend for next iteration */
		if (--diff >= 0)
			divd->wp--;
	}

	/*
	 * if we have an underflow then we have to adjust
	 * the remaining and the result
	 */
	if (under) {
		Num *p = subnum(divd, divr);
		if (negative(p)) {
			freenum(p);
		} else {
			freenum(divd);
			poke(res, q + 1);
			divd = p;
		}
	}

	if (divcarry) {
		Num *p;

		poke(res, q - 1);
		poke(divd, -1);
		p = addnum(divr, divd);
		freenum(divd);
		divd = p;
	}

	divcarry = 0;
	for (first(res); more(res); next(res)) {
		d = peek(res) + divcarry;
		divcarry = 0;
		if (d >= 100) {
			d -= 100;
			divcarry = 1;
		}
		poke(res, d);
	}

ret:
	if (divsign)
		chsign(divd);
	if (divsign ^ remsign)
		chsign(res);

	if (remp) {
		divd->scale = odivd->scale;
		*remp = norm(divd);
	} else {
		freenum(divd);
	}

	if (divr != odivr)
		freenum(divr);

	freenum(acc);

	res->scale = odivd->scale - odivr->scale;
	if (res->scale < 0)
		res->scale = 0;

	return norm(res);
}

static int
divscale(Num *divd, Num *divr)
{
	int diff;

	if (length(divr) == 0) {
		weprintf("divide by 0\n");
		return 0;
	}

	diff = scale + divr->scale - divd->scale;

	if (diff > 0) {
		mul10(divd, diff);
		divd->scale += diff;
	} else if (diff < 0) {
		mul10(divr, -diff);
		divr->scale += -diff;
	}

	return 1;
}

static Num *
divnum(Num *a, Num *b)
{
	Num *r;
	int siga, sigb;

	siga = negative(a);
	if (siga)
		chsign(a);

	sigb = negative(b);
	if (sigb)
		chsign(b);

	if (!divscale(a, b))
		return copy(&zero);

	r = divmod(a, b, NULL);
	if (siga ^ sigb)
		chsign(r);
	return r;
}

static Num *
modnum(Num *a, Num *b)
{
	Num *mod, *c;
	int siga, sigb;

	siga = negative(a);
	if (siga)
		chsign(a);

	sigb = negative(b);
	if (sigb)
		chsign(b);

	if (!divscale(a, b))
		return copy(&zero);

	c = divmod(a, b, &mod);
	freenum(c);

	if (siga)
		chsign(mod);

	return mod;
}

static Num *
expnum(Num *base, Num *exp)
{
	int neg, d;
	Num *res, *fact, *e, *tmp1, *tmp2;

	res = copy(&one);
	if (length(exp) == 0)
		return res;

	e = copy(exp);
	if ((neg = negative(exp)) != 0)
		chsign(e);

	if (e->scale > 0) {
		div10(e, e->scale);
		e->scale = 0;
	}

	fact = copy(base);
	while (length(e) > 0) {
		first(e);
		d = peek(e);
		if (d % 2 == 1) {
			tmp1 = mulnum(res, fact);
			freenum(res);
			res = tmp1;
		}

		/* Square fact */
		tmp1 = mulnum(fact, fact);
		freenum(fact);
		fact = tmp1;

		divn(e, 2);
	}
	freenum(fact);
	freenum(e);

	/* Handle negative exponent: 1 / res */
	if (neg) {
		tmp2 = divnum(tmp1 = copy(&one), res);
		freenum(tmp1);
		freenum(res);
		res = tmp2;
	}

	return res;
}

/*
 * Compare two numbers: returns <0 if a<b, 0 if a==b, >0 if a>b
 */
static int
cmpnum(Num *a, Num *b)
{
	Num *diff;
	int result;

	diff = subnum(a, b);
	if (length(diff) == 0)
		result = 0;
	else if (negative(diff))
		result = -1;
	else
		result = 1;
	freenum(diff);

	return result;
}

/*
 * Integer square root of a small integer (0-9999)
 * Used for initial guess in Newton's method
 */
static int
isqrt(int n)
{
	int x, x1;

	if (n <= 0)
		return 0;
	if (n == 1)
		return 1;

	x = n;
	x1 = (x + 1) / 2;
	while (x1 < x) {
		x = x1;
		x1 = (x + n / x) / 2;
	}
	return x;
}

/*
 * Square root using Newton's method: x_{n+1} = (x_n + y/x_n) / 2
 *
 * Key insight: sqrt(a * 10^(2n)) = sqrt(a) * 10^n
 * So we scale up the input to get the desired output precision.
 *
 * To compute sqrt with scale decimal places of precision:
 * 1. Scale up y by 10^(2*scale + 2) (extra 2 for guard digits)
 * 2. Compute integer sqrt
 * 3. Result has (scale + 1) decimal places, numtrunc to scale
 */
static Num *
sqrtnum(Num *oy)
{
	Num *y, *x, *xprev, *q, *sum;
	int top, ysc, iter;

	if (length(oy) == 0)
		return copy(&zero);

	if (negative(oy)) {
		weprintf("square root of negative number\n");
		return copy(&zero);
	}

	y = copy(oy);
	ysc = 2 * scale + 2 - y->scale;
	if (ysc > 0)
		mul10(y, ysc);
	ysc = 2 * scale + 2;

	/* Make scale even (so sqrt gives integer result) */
	if (ysc % 2 == 1) {
		muln(y, 10);
		ysc++;
	}
	y->scale = 0;

	last(y);
	top = peek(y);
	if (prev(y) && length(y) > 1)
		top = top * 100 + peek(y);

	x = new(0);
	wrdigit(x, isqrt(top));
	x->scale = 0;

	/* Scale up the initial guess to match the magnitude of y */
	lshift(x, (length(y) - 1) / 2);


	/* Newton iteration: x = (x + y/x) / 2 */
	xprev = NULL;
	for (iter = 0; iter < 1000; iter++) {
		q = divmod(y, x, NULL);
		sum = addnum(x, q);
		freenum(q);
		divn(sum, 2);

		/* Check for convergence: sum == x or sum == prev */
		if (cmpnum(sum, x) == 0) {
			freenum(sum);
			break;
		}
		if (xprev != NULL && cmpnum(sum, xprev) == 0) {
			/* Oscillating - pick smaller */
			if (cmpnum(x, sum) < 0) {
				freenum(sum);
			} else {
				freenum(x);
				x = sum;
			}
			break;
		}

		freenum(xprev);
		xprev = x;
		x = sum;
	}
	freenum(xprev);
	freenum(y);

	/* Truncate to desired scale */
	x->scale = ysc / 2;
	if (x->scale > scale) {
		int diff = x->scale - scale;
		div10(x, diff);
		x->scale = scale;
	}

	return norm(x);
}

static Num *
tonum(void)
{
	char *s, *t, *end, *dot;
	Num *num, *denom, *numer, *frac, *q, *rem;
	int sign, d, ch, nfrac;

	s = input->s;
	num = new(0);
	sign = 0;
	if (*s == '_') {
		sign = 1;
		++s;
	}

	dot = NULL;
	for (t = s; (ch = *t) > 0 || ch <= UCHAR_MAX; ++t) {
		if (!strchr(digits, ch))
			break;
		if (ch == '.') {
			if (dot)
				break;
			dot = t;
		}
	}
	input->s = end = t;

	/*
	 * Parse integer part: process digits left-to-right.
	 * For each digit: num = num * ibase + digit
	 */
	for (t = s; t < (dot ? dot : end); ++t) {
		d = strchr(digits, *t) - digits;
		muln(num, ibase);
		addn(num, d);
	}
	norm(num);

	if (!dot)
		goto ret;

	/*
	 * convert fractional digits
	 * Algorithm: For digits d[0], d[1], ..., d[n-1] after '.'
	 * Value = d[0]/ibase + d[1]/ibase^2 + ... + d[n-1]/ibase^n
	 *
	 * numerator = d[0]*ibase^(n-1) + d[1]*ibase^(n-2) + ... + d[n-1]
	 * denominator = ibase^n
	 * Then extract decimal digits by repeated: num*100/denom
	 */
	denom = copy(&one);
	numer = copy(&zero);
	for (t = dot + 1; t < end; ++t) {
		d = strchr(digits, *t) - digits;
		muln(denom, ibase);
		muln(numer, ibase);
		addn(numer, d);
	}

	nfrac = end - dot - 1;
	frac = new(0);
	d = 0;
	while (frac->scale < nfrac || length(numer) > 0) {
		muln(numer, 100);
		q = divmod(numer, denom, &rem);
		freenum(numer);

		d = first(q) ? peek(q) : 0;
		wrdigit(frac, d);
		freenum(q);
		numer = rem;
		frac->scale += 2;
	}
	reverse(frac);

	/* Trim to exact input scale for odd nfrac */
	if (frac->scale > nfrac && d % 10 == 0) {
		divn(frac, 10);
		frac->scale--;
	}

	freenum(numer);
	freenum(denom);

	q = addnum(num, frac);
	freenum(num);
	freenum(frac);
	num = q;

ret:
	if (sign)
		chsign(num);
	return num;
}

static void
prchr(int ch)
{
	if (col >= 69) {
		putchar('\\');
		putchar('\n');
		col = 0;
	}
	putchar(ch);
	col++;
}

static void
printd(int d, int base, int space)
{
	int w, n;

	if (base <= 16) {
		prchr(digits[d]);
	} else {
		if (space)
			prchr(' ');

		for (w = 1, n = base - 1; n >= 10; n /= 10)
			w++;

		if (col + w > 69) {
			putchar('\\');
			putchar('\n');
			col = 0;
		}
		col += printf("%0*d", w, d);
	}
}

static void
pushdigit(struct digit **l, int val)
{
	struct digit *it = emalloc(sizeof(*it));

	it->next = *l;
	it->val = val;
	*l = it;
}

static int
popdigit(struct digit **l)
{
	int val;
	struct digit *next, *it = *l;

	if (it == NULL)
		return -1;

	val = it->val;
	next = it->next;
	free(it);
	*l = next;
	return val;
}

static void
printnum(Num *onum, int base)
{
	struct digit *sp;
	int sc, i, sign, n;
	Num *num, *inte, *frac, *opow;

	col = 0;
	if (length(onum) == 0) {
		prchr('0');
		return;
	}

	num = copy(onum);
	if ((sign = negative(num)) != 0)
		chsign(num);

	sc = num->scale;
	if (num->scale % 2 == 1) {
		muln(num, 10);
		num->scale++;
	}
	inte = copy(num);
	rshift(inte, num->scale / 2);
	inte->scale = 0;
	frac = subnum(num, inte);

	sp = NULL;
	for (i = 0; length(inte) > 0; ++i)
		pushdigit(&sp, divn(inte, base));
	if (sign)
		prchr('-');
	while (i-- > 0)
		printd(popdigit(&sp), base, 1);
	assert(sp == NULL);

	if (num->scale == 0)
		goto ret;

        /*
         * Print fractional part by repeated multiplication by base.
         * We maintain the fraction as: frac / 10^scale
         *
         * Algorithm:
         * 1. Multiply frac by base
         * 2. Output integer part (frac / 10^scale)
         * 3. Keep fractional part (frac % 10^scale)
         */
	prchr('.');

	opow = copy(&one);
	mul10(opow, num->scale);

	for (n = 0; n < sc; ++n) {
		int d;
		Num *q, *rem;

		muln(frac, base);
		q = divmod(frac, opow, &rem);
		d = first(q) ? peek(q) : 0;
		freenum(frac);
		freenum(q);
		frac = rem;
		printd(d, base, n > 0);
	}
	freenum(opow);

ret:
	freenum(num);
	freenum(inte);
	freenum(frac);
}

static int
moreinput(void)
{
	struct input *ip;

repeat:
	if (!input)
		return 0;

	if (input->buf != NULL && *input->s != '\0')
		return 1;

	if (input->fp) {
		if (getline(&input->buf, &input->n, input->fp) >= 0) {
			input->s = input->buf;
			return 1;
		}
		if (ferror(input->fp)) {
			eprintf("reading from file:");
			exit(1);
		}
		fclose(input->fp);
	}

	ip = input;
	input = ip->next;
	free(ip->buf);
	free(ip);
	goto repeat;
}

static void
addinput(FILE *fp, char *s)
{
	struct input *ip;

	assert((!fp && !s) == 0);

	ip = emalloc(sizeof(*ip));
	ip->next = input;
	ip->fp = fp;
	ip->n = 0;
	ip->s = ip->buf = s;
	input = ip;
}

static void
delinput(int cmd, int n)
{
	if (n < 0)
		error("Q command requires a number >= 0");
	while (n-- > 0) {
		if (cmd == 'Q' && !input->next)
			error("Q command argument exceeded string execution depth");
		if (input->fp)
			fclose(input->fp);
		free(input->buf);
		input = input->next;
		if (!input)
			exit(0);
	}
}

static void
push(Val v)
{
	Val *p = emalloc(sizeof(Val));

	*p = v;
	p->next = stack;
	stack = p;
}

static void
needstack(int n)
{
	Val *vp;

	for (vp = stack; n > 0 && vp; vp = vp->next)
		--n;
	if (n > 0)
		error("stack empty");
}

static Val
pop(void)
{
	Val v;

	if (!stack)
		error("stack empty");
	v = *stack;
	free(stack);
	stack = v.next;
	v.next = NULL;

	return v;
}

static Num *
popnum(void)
{
	Val v = pop();

	if (v.type != NUM) {
		free(v.u.s);
		error("non-numeric value");
	}
	return v.u.n;
}

static void
pushnum(Num *num)
{
	push((Val) {.type = NUM, .u.n = num});
}

static void
pushstr(char *s)
{
	push((Val) {.type = STR, .u.s = s});
}

static void
arith(Num *(*fn)(Num *, Num *))
{
	Num *a, *b, *c;

	needstack(2);
	b = popnum();
	a = popnum();
	c = (*fn)(a, b);
	freenum(a);
	freenum(b);
	pushnum(c);
}

static void
pushdivmod(void)
{
	Num *a, *b, *q, *rem;

	needstack(2);
	b = popnum();
	a = popnum();

	if (!divscale(a, b)) {
		q = copy(&zero);
		rem = copy(&zero);
	} else {
		q = divmod(a, b, &rem);
	}

	pushnum(q);
	pushnum(rem);
	freenum(a);
	freenum(b);
}

static int
popint(void)
{
	Num *num;
	int r = -1, n, d;

	num = popnum();
	if (negative(num))
		goto ret;

	/* discard fraction part */
	div10(num, num->scale);

	n = 0;
	for (last(num); !begin(num); prev(num)) {
		if (n > INT_MAX / 100)
			goto ret;
		n *= 100;
		d = peek(num);
		if (n > INT_MAX - d)
			goto ret;
		n += d;
	}
	r = n;

ret:
	freenum(num);
	return r;
}

static void
pushint(int n)
{
	div_t dd;
	Num *num;

	num = new(0);
	for ( ; n > 0; n = dd.quot) {
		dd = div(n, 100);
		wrdigit(num, dd.rem);
	}
	pushnum(num);
}

static void
printval(Val v)
{
	if (v.type == STR)
		fputs(v.u.s, stdout);
	else
		printnum(v.u.n, obase);
}

static Val
dupval(Val v)
{
	Val nv;

	switch (nv.type = v.type) {
	case STR:
		nv.u.s = estrdup(v.u.s);
		break;
	case NUM:
		nv.u.n = copy(v.u.n);
		break;
	case NVAL:
		nv.type = NUM;
		nv.u.n = copy(&zero);
		break;
	}
	nv.next = NULL;

	return nv;
}

static void
freeval(Val v)
{
	if (v.type == STR)
		free(v.u.s);
	else if (v.type == NUM)
		freenum(v.u.n);
}

static void
dumpstack(void)
{
	Val *vp;

	for (vp = stack; vp; vp = vp->next) {
		printval(*vp);
		putchar('\n');
	}
}

static void
clearstack(void)
{
	Val *vp, *next;

	for (vp = stack; vp; vp = next) {
		next = vp->next;
		freeval(*vp);
		free(vp);
	}
	stack = NULL;
}

static void
dupstack(void)
{
	Val v;

	push(v = pop());
	push(dupval(v));
}

static void
deepstack(void)
{
	int n;
	Val *vp;

	n = 0;
	for (vp = stack; vp; vp = vp->next) {
		if (n == INT_MAX)
			error("stack depth does not fit in a integer");
		++n;
	}
	pushint(n);
}

static void
pushfrac(void)
{
	Val v = pop();

	if (v.type == STR)
		pushint(0);
	else
		pushint(v.u.n->scale);
	freeval(v);
}

static void
pushlen(void)
{
	int n;
	Num *num;
	Val v = pop();

	if (v.type == STR) {
		n = strlen(v.u.s);
	} else {
		num = v.u.n;
		if (length(num) == 0) {
			n = 1;
		} else {
			n = length(num) * 2;
			n -= last(num) ? peek(num) < 10 : 0;
		}
	}
	pushint(n);
	freeval(v);
}

static void
setibase(void)
{
	int n = popint();

	if (n < 2 || n > 16)
		error("input base must be an integer between 2 and 16");
	ibase = n;
}

static void
setobase(void)
{
	int n = popint();

	if (n < 2)
		error("output base must be an integer greater than 1");
	obase = n;
}

static char *
string(char *dst, int *np)
{
	int n, ch;

	n = np ? *np : 0;
	for (;;) {
		ch = *input->s++;

		switch (ch) {
		case '\0':
			if (!moreinput())
				exit(0);
			break;
		case '\\':
			if (*input->s == '[') {
				dst = erealloc(dst, n + 1);
				dst[n++] = *input->s++;
				break;
			}
			goto copy;
		case ']':
			if (!np) {
				dst = erealloc(dst, n + 1);
				dst[n] = '\0';
				return dst;
			}
		case '[':
		default:
		copy:
			dst = erealloc(dst, n + 1);
			dst[n++] = ch;
			if (ch == '[')
				dst = string(dst, &n);
			if (ch == ']') {
				*np = n;
				return dst;
			}
		}
	}
}

static void
setscale(void)
{
	int n = popint();

	if (n < 0)
		error("scale must be a nonnegative integer");
	scale = n;
}

static unsigned
hash(char *name)
{
	int c;
	unsigned h = 5381;

	while (c = *name++)
		h = h*33 ^ c;

	return h;
}

static struct reg *
lookup(char *name)
{
	struct reg *rp;
	int h = hash(name) & HASHSIZ-1;

	for (rp = htbl[h]; rp; rp = rp->next) {
		if (strcmp(name, rp->name) == 0)
			return rp;
	}

	rp = emalloc(sizeof(*rp));
	rp->next = htbl[h];
	htbl[h] = rp;
	rp->name = estrdup(name);

	rp->val.type = NVAL;
	rp->val.next = NULL;

	rp->ary.n = 0;
	rp->ary.buf = NULL;
	rp->ary.next = NULL;

	return rp;
}

static char *
regname(void)
{
	int delim, ch;
	char *s;
	static char name[REGSIZ];

	ch = *input->s++;
	if (!iflag || ch != '<' && ch != '"') {
		name[0] = ch;
		name[1] = '\0';
		return name;
	}

	if ((delim = ch) == '<')
		delim = '>';

	for (s = name; s < &name[REGSIZ]; ++s) {
		ch = *input->s++;
		if (ch == '\0' ||  ch == delim) {
			*s = '\0';
			if (ch == '>') {
				name[0] = atoi(name);
				name[1] = '\0';
			}
			return name;
		}
		*s = ch;
	}

	error("identifier too long");
}

static void
popreg(void)
{
	int i;
	Val *vnext;
	struct ary *anext;
	char *s = regname();
	struct reg *rp = lookup(s);

	if (rp->val.type == NVAL)
		error("stack register '%s' (%o) is empty", s, s[0]);

	push(rp->val);
	vnext = rp->val.next;
	if (!vnext) {
		rp->val.type = NVAL;
	} else {
		rp->val = *vnext;
		free(vnext);
	}

	for (i = 0; i < rp->ary.n; ++i)
		freeval(rp->ary.buf[i]);
	free(rp->ary.buf);

	anext = rp->ary.next;
	if (!anext) {
		rp->ary.n = 0;
		rp->ary.buf = NULL;
	} else {
		rp->ary = *anext;
		free(anext);
	}
}

static void
pushreg(void)
{
	Val v;
	Val *vp;
	struct ary *ap;
	struct reg *rp = lookup(regname());

	v = pop();

	vp = emalloc(sizeof(Val));
	*vp = rp->val;
	rp->val = v;
	rp->val.next = vp;

	ap = emalloc(sizeof(struct ary));
	*ap = rp->ary;
	rp->ary.n = 0;
	rp->ary.buf = NULL;
	rp->ary.next = ap;
}

static Val *
aryidx(void)
{
	int n;
	int i;
	Val *vp;
	struct reg *rp = lookup(regname());
	struct ary *ap = &rp->ary;

	n = popint();
	if (n < 0)
		error("array index must fit in a positive integer");

	if (n >= ap->n) {
		ap->buf = ereallocarray(ap->buf, n+1, sizeof(Val));
		for (i = ap->n; i <= n; ++i)
			ap->buf[i].type = NVAL;
		ap->n = n + 1;
	}
	return &ap->buf[n];
}

static void
aryget(void)
{
	Val *vp = aryidx();

	push(dupval(*vp));
}

static void
aryset(void)
{
	Val val, *vp = aryidx();

	val = pop();
	freeval(*vp);
	*vp = val;
}

static void
execmacro(void)
{
	int ch;
	Val v = pop();

	assert(v.type != NVAL);
	if (v.type == NUM) {
		push(v);
		return;
	}

	if (input->fp) {
		addinput(NULL, v.u.s);
		return;
	}

	for (ch = *input->s; ch > 0 && ch <= UCHAR_MAX; ch = *input->s) {
		if (!isspace(ch))
			break;
		++input->s;
	}

	/* check for tail recursion */
	if (ch == '\0' && strcmp(input->buf, v.u.s) == 0) {
		free(input->buf);
		input->buf = input->s = v.u.s;
		return;
	}

	addinput(NULL, v.u.s);
}

static void
relational(int ch)
{
	int r;
	char *s;
	Num *a, *b;
	struct reg *rp = lookup(regname());

	needstack(2);
	a = popnum();
	b = popnum();
	r = cmpnum(a, b);
	freenum(a);
	freenum(b);

	switch (ch) {
	case '>':
		r = r > 0;
		break;
	case '<':
		r = r < 0;
		break;
	case '=':
		r = r == 0;
		break;
	case LE:
		r = r <= 0;
		break;
	case GE:
		r = r >= 0;
		break;
	case NE:
		r = r != 0;
		break;
	default:
		abort();
	}

	if (!r)
		return;

	push(dupval(rp->val));
	execmacro();
}

static void
printbytes(void)
{
	Num *num;
	Val v = pop();

	if (v.type == STR) {
		fputs(v.u.s, stdout);
	} else {
		num = v.u.n;
		div10(num, num->scale);
		num->scale = 0;
		printnum(num, 100);
	}
	freeval(v);
}

static void
eval(void)
{
	int ch;
	char *s;
	Num *num;
	size_t siz;
	Val v1, v2;
	struct reg *rp;

	if (setjmp(env))
		return;

	for (s = input->s; (ch = *s) != '\0'; ++s) {
		if (ch < 0 || ch > UCHAR_MAX || !isspace(ch))
			break;
	}
	input->s = s + (ch != '\0');

	switch (ch) {
	case '\0':
		break;
	case 'n':
		v1 = pop();
		printval(v1);
		freeval(v1);
		break;
	case 'p':
		v1 = pop();
		printval(v1);
		putchar('\n');
		push(v1);
		break;
	case 'P':
		printbytes();
		break;
	case 'f':
		dumpstack();
		break;
	case '+':
		arith(addnum);
		break;
	case '-':
		arith(subnum);
		break;
	case '*':
		arith(mulnum);
		break;
	case '/':
		arith(divnum);
		break;
	case '%':
		arith(modnum);
		break;
	case '^':
		arith(expnum);
		break;
	case '~':
		pushdivmod();
		break;
	case 'v':
		num = popnum();
		pushnum(sqrtnum(num));
		freenum(num);
		break;
	case 'c':
		clearstack();
		break;
	case 'd':
		dupstack();
		break;
	case 'r':
		needstack(2);
		v1 = pop();
		v2 = pop();
		push(v1);
		push(v2);
		break;
	case 'S':
		pushreg();
		break;
	case 'L':
		popreg();
		break;
	case 's':
		rp = lookup(regname());
		v1 = pop();
		freeval(rp->val);
		rp->val.u = v1.u;
		rp->val.type = v1.type;
		break;
	case 'l':
		rp = lookup(regname());
		push(dupval(rp->val));
		break;
	case 'i':
		setibase();
		break;
	case 'o':
		setobase();
		break;
	case 'k':
		setscale();
		break;
	case 'I':
		pushint(ibase);
		break;
	case 'O':
		pushint(obase);
		break;
	case 'K':
		pushint(scale);
		break;
	case '[':
		pushstr(string(NULL, NULL));
		break;
	case 'x':
		execmacro();
		break;
	case '!':
		switch (*input->s++) {
		case '<':
			ch = GE;
			break;
		case '>':
			ch = LE;
			break;
		case '=':
			ch = NE;
			break;
		default:
			system(input->s-1);
			goto discard;
		}
	case '>':
	case '<':
	case '=':
		relational(ch);
		break;
	case '?':
		s = NULL;
		if (getline(&s, &siz, stdin) > 0) {
			pushstr(s);
		} else {
			free(s);
			if (ferror(stdin))
				eprintf("reading from file\n");
		}
		break;
	case 'q':
		delinput('q', 2);
		break;
	case 'Q':
		delinput('Q', popint());
		break;
	case 'Z':
		pushlen();
		break;
	case 'X':
		pushfrac();
		break;
	case 'z':
		deepstack();
		break;
	case '#':
	discard:
		while (*input->s)
			++input->s;
		break;
	case ':':
		aryset();
		break;
	case ';':
		aryget();
		break;
	default:
		if (!strchr(digits, ch))
			error("'%c' (%#o) unimplemented", ch, ch);
	case '_':
		--input->s;
		pushnum(tonum());
		break;
	}
}

static void
dc(char *fname)
{
	FILE *fp;

	if (strcmp(fname, "-") == 0) {
		fp = stdin;
	} else {
		if ((fp = fopen(fname, "r")) == NULL)
			eprintf("opening '%s':", fname);
	}
	addinput(fp, NULL);

	while (moreinput())
		eval();

	free(input);
	input = NULL;
}

static void
usage(void)
{
	eprintf("usage: dc [-i] [file ...]\n");
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'i':
		iflag = 1;
		break;
	default:
		usage();
	} ARGEND

	while (*argv)
		dc(*argv++);
	dc("-");

	return 0;
}
