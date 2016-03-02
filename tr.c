/* See LICENSE file for copyright and license details. */
#include <stdlib.h>

#include "utf.h"
#include "util.h"

static int cflag = 0;
static int dflag = 0;
static int sflag = 0;

struct range {
	Rune   start;
	Rune   end;
	size_t quant;
};

static struct {
	char    *name;
	int    (*check)(Rune);
} classes[] = {
	{ "alnum",  isalnumrune  },
	{ "alpha",  isalpharune  },
	{ "blank",  isblankrune  },
	{ "cntrl",  iscntrlrune  },
	{ "digit",  isdigitrune  },
	{ "graph",  isgraphrune  },
	{ "lower",  islowerrune  },
	{ "print",  isprintrune  },
	{ "punct",  ispunctrune  },
	{ "space",  isspacerune  },
	{ "upper",  isupperrune  },
	{ "xdigit", isxdigitrune },
};

static struct range *set1        = NULL;
static size_t set1ranges         = 0;
static int    (*set1check)(Rune) = NULL;
static struct range *set2        = NULL;
static size_t set2ranges         = 0;
static int    (*set2check)(Rune) = NULL;

static size_t
rangelen(struct range r)
{
	return (r.end - r.start + 1) * r.quant;
}

static size_t
setlen(struct range *set, size_t setranges)
{
	size_t len = 0, i;

	for (i = 0; i < setranges; i++)
		len += rangelen(set[i]);

	return len;
}

static int
rstrmatch(Rune *r, char *s, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++)
		if (r[i] != s[i])
			return 0;
	return 1;
}

static size_t
makeset(char *str, struct range **set, int (**check)(Rune))
{
	Rune  *rstr;
	size_t len, i, j, m, n;
	size_t q, setranges = 0;
	int    factor, base;

	/* rstr defines at most len ranges */
	unescape(str);
	rstr = ereallocarray(NULL, utflen(str) + 1, sizeof(*rstr));
	len = utftorunestr(str, rstr);
	*set = ereallocarray(NULL, len, sizeof(**set));

	for (i = 0; i < len; i++) {
		if (rstr[i] == '[') {
			j = i;
nextbrack:
			if (j == len)
				goto literal;
			for (m = j; m < len; m++)
				if (rstr[m] == ']') {
					j = m;
					break;
				}
			if (j == i)
				goto literal;

			/* CLASSES [=EQUIV=] (skip) */
			if (j - i > 3 && rstr[i + 1] == '=' && rstr[m - 1] == '=') {
				if (j - i != 4)
					goto literal;
				(*set)[setranges].start = rstr[i + 2];
				(*set)[setranges].end   = rstr[i + 2];
				(*set)[setranges].quant = 1;
				setranges++;
				i = j;
				continue;
			}

			/* CLASSES [:CLASS:] */
			if (j - i > 3 && rstr[i + 1] == ':' && rstr[m - 1] == ':') {
				for (n = 0; n < LEN(classes); n++) {
					if (rstrmatch(rstr + i + 2, classes[n].name, j - i - 3)) {
						*check = classes[n].check;
						return 0;
					}
				}
				eprintf("Invalid character class.\n");
			}

			/* REPEAT  [_*n] (only allowed in set2) */
			if (j - i > 2 && rstr[i + 2] == '*' && set1ranges > 0) {
				/* check if right side of '*' is a number */
				q = 0;
				factor = 1;
				base = (rstr[i + 3] == '0') ? 8 : 10;
				for (n = j - 1; n > i + 2; n--) {
					if (rstr[n] < '0' || rstr[n] > '9') {
						n = 0;
						break;
					}
					q += (rstr[n] - '0') * factor;
					factor *= base;
				}
				if (n == 0) {
					j = m + 1;
					goto nextbrack;
				}
				(*set)[setranges].start = rstr[i + 1];
				(*set)[setranges].end   = rstr[i + 1];
				(*set)[setranges].quant = q ? q : setlen(set1, set1ranges);
				setranges++;
				i = j;
				continue;
			}

			j = m + 1;
			goto nextbrack;
		}
literal:
		/* RANGES [_-__-_], _-__-_ */
		/* LITERALS _______ */
		(*set)[setranges].start = rstr[i];

		if (i < len - 2 && rstr[i + 1] == '-' && rstr[i + 2] >= rstr[i])
			i += 2;
		(*set)[setranges].end = rstr[i];
		(*set)[setranges].quant = 1;
		setranges++;
	}

	free(rstr);
	return setranges;
}

static void
usage(void)
{
	eprintf("usage: %s [-cCds] set1 [set2]\n", argv0);
}

int
main(int argc, char *argv[])
{
	Rune r = 0, lastrune = 0;
	size_t off1, off2, i, m;
	int ret = 0;

	ARGBEGIN {
	case 'c':
	case 'C':
		cflag = 1;
		break;
	case 'd':
		dflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (!argc || argc > 2 || (argc == 1 && dflag == sflag))
		usage();
	set1ranges = makeset(argv[0], &set1, &set1check);
	if (argc == 2)
		set2ranges = makeset(argv[1], &set2, &set2check);
	if (dflag == sflag && !set2ranges && !set2check)
		eprintf("set2 must be non-empty.\n");
	if (argc == 2 && !set2check != !set1check)
		eprintf("can't mix classes with non-classes.\n");
	if (set2check && set2check != islowerrune && set2check != isupperrune)
		eprintf("set2 can only be the 'lower' or 'upper' class.\n");
	if (set2check && cflag && !dflag)
		eprintf("set2 can't be imaged to from a complement.\n");
read:
	if (!efgetrune(&r, stdin, "<stdin>")) {
		ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");
		return ret;
	}
	off1 = off2 = 0;
	for (i = 0; i < set1ranges; i++) {
		if (set1[i].start <= r && r <= set1[i].end) {
			if (dflag) {
				if (!cflag || (sflag && r == lastrune))
					goto read;
				else
					goto write;
			}
			if (cflag)
				goto write;
			for (m = 0; m < i; m++)
				off1 += rangelen(set1[m]);
			off1 += r - set1[m].start;
			if (off1 > setlen(set2, set2ranges) - 1) {
				r = set2[set2ranges - 1].end;
				goto write;
			}
			for (m = 0; m < set2ranges; m++) {
				if (off2 + rangelen(set2[m]) > off1) {
					m++;
					break;
				}
				off2 += rangelen(set2[m]);
			}
			m--;
			r = set2[m].start + (off1 - off2) / set2[m].quant;

			if (sflag && (r == lastrune))
				goto read;
			goto write;
		}
	}
	if (set1check && set1check(r)) {
		if (dflag) {
			if (!cflag || (sflag && r == lastrune))
				goto read;
			else
				goto write;
		}
		if (sflag) {
			if (r == lastrune)
				goto read;
			else
				goto write;
		}
		if (set1check == isupperrune && set2check == islowerrune)
			r = tolowerrune(r);
		else if (set1check == islowerrune && set2check == isupperrune)
			r = toupperrune(r);
		else if (set2ranges > 0)
			r = cflag ? r : set2[set2ranges - 1].end;
		else
			eprintf("Misaligned character classes.\n");
	} else if (cflag && set2ranges > 0) {
		r = set2[set2ranges - 1].end;
	}
	if (dflag && cflag)
		goto read;
	if (dflag && sflag && r == lastrune)
		goto read;
write:
	lastrune = r;
	efputrune(&r, stdout, "<stdout>");
	goto read;
}
