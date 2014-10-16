/*	$OpenBSD: src/bin/expr/expr.c,v 1.19 2013/11/21 15:54:45 deraadt Exp $	*/
/*	$NetBSD: expr.c,v 1.3.6.1 1996/06/04 20:41:47 cgd Exp $	*/

/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <ctype.h>
#include <regex.h>
#include <err.h>

static struct val	*make_int(int);
static struct val	*make_str(char *);
static void		free_value(struct val *);
static int		is_integer(struct val *, int *);
static int		to_integer(struct val *);
static void		to_string(struct val *);
static int		is_zero_or_null(struct val *);
static void		nexttoken(int);
static void		error(void);
static struct val	*eval6(void);
static struct val	*eval5(void);
static struct val	*eval4(void);
static struct val	*eval3(void);
static struct val	*eval2(void);
static struct val	*eval1(void);
static struct val	*eval0(void);

enum token {
	OR, AND, EQ, LT, GT, ADD, SUB, MUL, DIV, MOD, MATCH, RP, LP,
	NE, LE, GE, OPERAND, EOI
};

struct val {
	enum {
		integer,
		string
	} type;

	union {
		char	       *s;
		int		i;
	} u;
};

static enum token	token;
static struct val     *tokval;
static char	      **av;

static struct val *
make_int(int i)
{
	struct val     *vp;

	vp = (struct val *) malloc(sizeof(*vp));
	if (vp == NULL) {
		err(3, NULL);
	}
	vp->type = integer;
	vp->u.i = i;
	return vp;
}

static struct val *
make_str(char *s)
{
	struct val     *vp;

	vp = (struct val *) malloc(sizeof(*vp));
	if (vp == NULL || ((vp->u.s = strdup(s)) == NULL)) {
		err(3, NULL);
	}
	vp->type = string;
	return vp;
}

static void
free_value(struct val *vp)
{
	if (vp->type == string)
		free(vp->u.s);
	free(vp);
}

/* determine if vp is an integer; if so, return it's value in *r */
static int
is_integer(struct val *vp, int *r)
{
	char	       *s;
	int		neg;
	int		i;

	if (vp->type == integer) {
		*r = vp->u.i;
		return 1;
	}

	/*
	 * POSIX.2 defines an "integer" as an optional unary minus
	 * followed by digits.
	 */
	s = vp->u.s;
	i = 0;

	neg = (*s == '-');
	if (neg)
		s++;

	while (*s) {
		if (!isdigit((unsigned char)*s))
			return 0;

		i *= 10;
		i += *s - '0';

		s++;
	}

	if (neg)
		i *= -1;

	*r = i;
	return 1;
}

/* coerce to vp to an integer */
static int
to_integer(struct val *vp)
{
	int		r;

	if (vp->type == integer)
		return 1;

	if (is_integer(vp, &r)) {
		free(vp->u.s);
		vp->u.i = r;
		vp->type = integer;
		return 1;
	}

	return 0;
}

/* coerce to vp to an string */
static void
to_string(struct val *vp)
{
	char	       *tmp;

	if (vp->type == string)
		return;

	if (asprintf(&tmp, "%d", vp->u.i) == -1)
		err(3, NULL);

	vp->type = string;
	vp->u.s = tmp;
}

static int
is_zero_or_null(struct val *vp)
{
	if (vp->type == integer) {
		return (vp->u.i == 0);
	} else {
		return (*vp->u.s == 0 || (to_integer(vp) && vp->u.i == 0));
	}
	/* NOTREACHED */
}

static void
nexttoken(int pat)
{
	char	       *p;

	if ((p = *av) == NULL) {
		token = EOI;
		return;
	}
	av++;

	if (pat == 0 && p[0] != '\0') {
		if (p[1] == '\0') {
			const char     *x = "|&=<>+-*/%:()";
			char	       *i;	/* index */

			if ((i = strchr(x, *p)) != NULL) {
				token = i - x;
				return;
			}
		} else if (p[1] == '=' && p[2] == '\0') {
			switch (*p) {
			case '<':
				token = LE;
				return;
			case '>':
				token = GE;
				return;
			case '!':
				token = NE;
				return;
			}
		}
	}
	tokval = make_str(p);
	token = OPERAND;
	return;
}

static void
error(void)
{
	errx(2, "syntax error");
	/* NOTREACHED */
}

static struct val *
eval6(void)
{
	struct val     *v;

	if (token == OPERAND) {
		nexttoken(0);
		return tokval;

	} else if (token == RP) {
		nexttoken(0);
		v = eval0();

		if (token != LP) {
			error();
			/* NOTREACHED */
		}
		nexttoken(0);
		return v;
	} else {
		error();
	}
	/* NOTREACHED */
	return NULL;
}

/* Parse and evaluate match (regex) expressions */
static struct val *
eval5(void)
{
	regex_t		rp;
	regmatch_t	rm[2];
	char		errbuf[256];
	int		eval;
	struct val     *l, *r;
	struct val     *v;

	l = eval6();
	while (token == MATCH) {
		nexttoken(1);
		r = eval6();

		/* coerce to both arguments to strings */
		to_string(l);
		to_string(r);

		/* compile regular expression */
		if ((eval = regcomp(&rp, r->u.s, 0)) != 0) {
			regerror(eval, &rp, errbuf, sizeof(errbuf));
			errx(2, "%s", errbuf);
		}

		/* compare string against pattern --  remember that patterns
		   are anchored to the beginning of the line */
		if (regexec(&rp, l->u.s, 2, rm, 0) == 0 && rm[0].rm_so == 0) {
			if (rm[1].rm_so >= 0) {
				*(l->u.s + rm[1].rm_eo) = '\0';
				v = make_str(l->u.s + rm[1].rm_so);

			} else {
				v = make_int((int)(rm[0].rm_eo - rm[0].rm_so));
			}
		} else {
			if (rp.re_nsub == 0) {
				v = make_int(0);
			} else {
				v = make_str("");
			}
		}

		/* free arguments and pattern buffer */
		free_value(l);
		free_value(r);
		regfree(&rp);

		l = v;
	}

	return l;
}

/* Parse and evaluate multiplication and division expressions */
static struct val *
eval4(void)
{
	struct val     *l, *r;
	enum token	op;

	l = eval5();
	while ((op = token) == MUL || op == DIV || op == MOD) {
		nexttoken(0);
		r = eval5();

		if (!to_integer(l) || !to_integer(r)) {
			errx(2, "non-numeric argument");
		}

		if (op == MUL) {
			l->u.i *= r->u.i;
		} else {
			if (r->u.i == 0) {
				errx(2, "division by zero");
			}
			if (op == DIV) {
				if (l->u.i != INT_MIN || r->u.i != -1)
					l->u.i /= r->u.i;
			} else {
				if (l->u.i != INT_MIN || r->u.i != -1)
					l->u.i %= r->u.i;
				else
					l->u.i = 0;
			}
		}

		free_value(r);
	}

	return l;
}

/* Parse and evaluate addition and subtraction expressions */
static struct val *
eval3(void)
{
	struct val     *l, *r;
	enum token	op;

	l = eval4();
	while ((op = token) == ADD || op == SUB) {
		nexttoken(0);
		r = eval4();

		if (!to_integer(l) || !to_integer(r)) {
			errx(2, "non-numeric argument");
		}

		if (op == ADD) {
			l->u.i += r->u.i;
		} else {
			l->u.i -= r->u.i;
		}

		free_value(r);
	}

	return l;
}

/* Parse and evaluate comparison expressions */
static struct val *
eval2(void)
{
	struct val     *l, *r;
	enum token	op;
	int		v = 0, li, ri;

	l = eval3();
	while ((op = token) == EQ || op == NE || op == LT || op == GT ||
	    op == LE || op == GE) {
		nexttoken(0);
		r = eval3();

		if (is_integer(l, &li) && is_integer(r, &ri)) {
			switch (op) {
			case GT:
				v = (li >  ri);
				break;
			case GE:
				v = (li >= ri);
				break;
			case LT:
				v = (li <  ri);
				break;
			case LE:
				v = (li <= ri);
				break;
			case EQ:
				v = (li == ri);
				break;
			case NE:
				v = (li != ri);
				break;
			default:
				break;
			}
		} else {
			to_string(l);
			to_string(r);

			switch (op) {
			case GT:
				v = (strcoll(l->u.s, r->u.s) > 0);
				break;
			case GE:
				v = (strcoll(l->u.s, r->u.s) >= 0);
				break;
			case LT:
				v = (strcoll(l->u.s, r->u.s) < 0);
				break;
			case LE:
				v = (strcoll(l->u.s, r->u.s) <= 0);
				break;
			case EQ:
				v = (strcoll(l->u.s, r->u.s) == 0);
				break;
			case NE:
				v = (strcoll(l->u.s, r->u.s) != 0);
				break;
			default:
				break;
			}
		}

		free_value(l);
		free_value(r);
		l = make_int(v);
	}

	return l;
}

/* Parse and evaluate & expressions */
static struct val *
eval1(void)
{
	struct val     *l, *r;

	l = eval2();
	while (token == AND) {
		nexttoken(0);
		r = eval2();

		if (is_zero_or_null(l) || is_zero_or_null(r)) {
			free_value(l);
			free_value(r);
			l = make_int(0);
		} else {
			free_value(r);
		}
	}

	return l;
}

/* Parse and evaluate | expressions */
static struct val *
eval0(void)
{
	struct val     *l, *r;

	l = eval1();
	while (token == OR) {
		nexttoken(0);
		r = eval1();

		if (is_zero_or_null(l)) {
			free_value(l);
			l = r;
		} else {
			free_value(r);
		}
	}

	return l;
}


int
main(int argc, char *argv[])
{
	struct val     *vp;

	(void) setlocale(LC_ALL, "");

	if (argc > 1 && !strcmp(argv[1], "--"))
		argv++;

	av = argv + 1;

	nexttoken(0);
	vp = eval0();

	if (token != EOI) {
		error();
		/* NOTREACHED */
	}

	if (vp->type == integer)
		printf("%d\n", vp->u.i);
	else
		printf("%s\n", vp->u.s);

	exit(is_zero_or_null(vp));
}
