/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "text.h"
#include "utf.h"
#include "util.h"

struct keydef {
	int start_column;
	int end_column;
	int start_char;
	int end_char;
	int flags;
	TAILQ_ENTRY(keydef) entry;
};

enum {
	MOD_N      = 1 << 0,
	MOD_STARTB = 1 << 1,
	MOD_ENDB   = 1 << 2,
	MOD_R      = 1 << 3,
	MOD_D      = 1 << 4,
	MOD_F      = 1 << 5,
	MOD_I      = 1 << 6,
};

static TAILQ_HEAD(kdhead, keydef) kdhead = TAILQ_HEAD_INITIALIZER(kdhead);

static int Cflag = 0, cflag = 0, uflag = 0;
static char *fieldsep = NULL;
static size_t fieldseplen = 0;
static char *col1, *col2;
static size_t col1siz, col2siz;

static char *
skipblank(char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;

	return s;
}

static char *
skipnonblank(char *s)
{
	while (*s && *s != '\n' && *s != ' ' && *s != '\t')
		s++;

	return s;
}

static char *
skipcolumn(char *s, char *eol, int skip_to_next_col)
{
	if (fieldsep) {
		if ((s = strstr(s, fieldsep))) {
			if (skip_to_next_col)
				s += fieldseplen;
		} else {
			s = eol;
		}
	} else {
		s = skipblank(s);
		s = skipnonblank(s);
	}

	return s;
}

static size_t
columns(char *line, const struct keydef *kd, char **col, size_t *colsiz)
{
	Rune r;
	char *start, *end, *eol = strchr(line, '\n');
	size_t len, utflen, rlen;
	int i;

	for (i = 1, start = line; i < kd->start_column; i++)
		start = skipcolumn(start, eol, 1);
	if (kd->flags & MOD_STARTB)
		start = skipblank(start);
	for (utflen = 0; start < eol && utflen < kd->start_char - 1;) {
		rlen = chartorune(&r, start);
		start += rlen;
		utflen++;
	}

	if (kd->end_column) {
		for (i = 1, end = line; i < kd->end_column; i++)
			end = skipcolumn(end, eol, 1);
		if (kd->flags & MOD_ENDB)
			end = skipblank(end);
		if (kd->end_char) {
			for (utflen = 0; end < eol && utflen < kd->end_char;) {
				rlen = chartorune(&r, end);
				end += rlen;
				utflen++;
			}
		} else {
			end = skipcolumn(end, eol, 0);
		}
	} else {
		end = eol;
	}
	len = (start > end) ? 0 : (end - start);
	if (!*col || *colsiz < len)
		*col = erealloc(*col, len + 1);
	memcpy(*col, start, len);
	(*col)[len] = '\0';
	if (*colsiz < len)
		*colsiz = len;

	return len;
}

static int
skipmodcmp(const char *s1, const char *s2, int flags)
{
	Rune r1, r2;

	do {
		s1 += chartorune(&r1, s1);
		s2 += chartorune(&r2, s2);

		if (flags & MOD_D && flags & MOD_I) {
			while (*s1 && ((!isblankrune(r1) && !isalnumrune(r1)) ||
						   (!isprintrune(r1))))
				s1 += chartorune(&r1, s1);
			while (*s2 && ((!isblankrune(r2) && !isalnumrune(r2)) ||
						   (!isprintrune(r2))))
				s2 += chartorune(&r2, s2);
		}
		else if (flags & MOD_D) {
			while (*s1 && !isblankrune(r1) && !isalnumrune(r1))
				s1 += chartorune(&r1, s1);
			while (*s2 && !isblankrune(r2) && !isalnumrune(r2))
				s2 += chartorune(&r2, s2);
		}
		else if (flags & MOD_I) {
			while (*s1 && !isprintrune(r1))
				s1 += chartorune(&r1, s1);
			while (*s2 && !isprintrune(r2))
				s2 += chartorune(&r2, s2);
		}
		if (flags & MOD_F) {
			r1 = toupperrune(r1);
			r2 = toupperrune(r2);
		}
	} while (r1 && r1 == r2);

	return r1 - r2;
}

static int
linecmp(const char **a, const char **b)
{
	int res = 0;
	long double x, y;
	struct keydef *kd;

	TAILQ_FOREACH(kd, &kdhead, entry) {
		columns((char *)*a, kd, &col1, &col1siz);
		columns((char *)*b, kd, &col2, &col2siz);

		/* if -u is given, don't use default key definition
		 * unless it is the only one */
		if (uflag && kd == TAILQ_LAST(&kdhead, kdhead) &&
		    TAILQ_LAST(&kdhead, kdhead) != TAILQ_FIRST(&kdhead)) {
			res = 0;
		} else if (kd->flags & MOD_N) {
			x = strtold(col1, NULL);
			y = strtold(col2, NULL);
			res = (x < y) ? -1 : (x > y);
		} else if (kd->flags & (MOD_D | MOD_F | MOD_I)) {
			res = skipmodcmp(col1, col2, kd->flags);
		} else {
			res = strcmp(col1, col2);
		}

		if (kd->flags & MOD_R)
			res = -res;
		if (res)
			break;
	}

	return res;
}

static int
check(FILE *fp, const char *fname)
{
	static struct { char *buf; size_t size; } prev, cur, tmp;

	if (!prev.buf && getline(&prev.buf, &prev.size, fp) < 0)
		eprintf("getline:");
	while (getline(&cur.buf, &cur.size, fp) > 0) {
		if (uflag > linecmp((const char **)&cur.buf,
		                    (const char **)&prev.buf)) {
			if (!Cflag)
				weprintf("disorder %s: %s", fname, cur.buf);
			return 1;
		}
		tmp = cur;
		cur = prev;
		prev = tmp;
	}

	return 0;
}

static int
parse_flags(char **s, int *flags, int bflag)
{
	while (isalpha((int)**s)) {
		switch (*((*s)++)) {
		case 'b':
			*flags |= bflag;
			break;
		case 'd':
			*flags |= MOD_D;
			break;
		case 'f':
			*flags |= MOD_F;
			break;
		case 'i':
			*flags |= MOD_I;
			break;
		case 'n':
			*flags |= MOD_N;
			break;
		case 'r':
			*flags |= MOD_R;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

static void
addkeydef(char *kdstr, int flags)
{
	struct keydef *kd;

	kd = enmalloc(2, sizeof(*kd));

	/* parse key definition kdstr with format
	 * start_column[.start_char][flags][,end_column[.end_char][flags]]
	 */
	kd->start_column = 1;
	kd->start_char = 1;
	kd->end_column = 0; /* 0 means end of line */
	kd->end_char = 0;   /* 0 means end of column */
	kd->flags = flags;

	if ((kd->start_column = strtol(kdstr, &kdstr, 10)) < 1)
		enprintf(2, "invalid start column in key definition\n");

	if (*kdstr == '.') {
		if ((kd->start_char = strtol(kdstr + 1, &kdstr, 10)) < 1)
			enprintf(2, "invalid start character in key "
			         "definition\n");
	}
	if (parse_flags(&kdstr, &kd->flags, MOD_STARTB) < 0)
		enprintf(2, "invalid start flags in key definition\n");

	if (*kdstr == ',') {
		if ((kd->end_column = strtol(kdstr + 1, &kdstr, 10)) < 0)
			enprintf(2, "invalid end column in key definition\n");
		if (*kdstr == '.') {
			if ((kd->end_char = strtol(kdstr + 1, &kdstr, 10)) < 0)
				enprintf(2, "invalid end character in key "
				         "definition\n");
		}
		if (parse_flags(&kdstr, &kd->flags, MOD_ENDB) < 0)
			enprintf(2, "invalid end flags in key definition\n");
	}

	if (*kdstr != '\0')
		enprintf(2, "invalid key definition\n");

	TAILQ_INSERT_TAIL(&kdhead, kd, entry);
}

static void
usage(void)
{
	enprintf(2, "usage: %s [-Cbcdfimnru] [-o outfile] [-t delim] "
	         "[-k def]... [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp, *ofp = stdout;
	struct linebuf linebuf = EMPTY_LINEBUF;
	size_t i;
	int global_flags = 0, ret = 0;
	char *outfile = NULL;

	ARGBEGIN {
	case 'C':
		Cflag = 1;
		break;
	case 'b':
		global_flags |= MOD_STARTB | MOD_ENDB;
		break;
	case 'c':
		cflag = 1;
		break;
	case 'd':
		global_flags |= MOD_D;
		break;
	case 'f':
		global_flags |= MOD_F;
		break;
	case 'i':
		global_flags |= MOD_I;
		break;
	case 'k':
		addkeydef(EARGF(usage()), global_flags);
		break;
	case 'm':
		/* more or less for free, but for performance-reasons,
		 * we should keep this flag in mind and maybe some later
		 * day implement it properly so we don't run out of memory
		 * while merging large sorted files.
		 */
		break;
	case 'n':
		global_flags |= MOD_N;
		break;
	case 'o':
		outfile = EARGF(usage());
		break;
	case 'r':
		global_flags |= MOD_R;
		break;
	case 't':
		fieldsep = EARGF(usage());
		fieldseplen = strlen(fieldsep);
		break;
	case 'u':
		uflag = 1;
		break;
	default:
		usage();
	} ARGEND

	/* -b shall only apply to custom key definitions */
	if (TAILQ_EMPTY(&kdhead) && global_flags)
		addkeydef("1", global_flags & ~(MOD_STARTB | MOD_ENDB));
	addkeydef("1", global_flags & MOD_R);

	if (!argc) {
		if (Cflag || cflag) {
			if (check(stdin, "<stdin>") && !ret)
				ret = 1;
		} else {
			getlines(stdin, &linebuf);
		}
	} else for (; *argv; argc--, argv++) {
		if (!strcmp(*argv, "-")) {
			*argv = "<stdin>";
			fp = stdin;
		} else if (!(fp = fopen(*argv, "r"))) {
			enprintf(2, "fopen %s:", *argv);
			continue;
		}
		if (Cflag || cflag) {
			if (check(fp, *argv) && !ret)
				ret = 1;
		} else {
			getlines(fp, &linebuf);
		}
		if (fp != stdin && fshut(fp, *argv))
			ret = 2;
	}

	if (!Cflag && !cflag) {
		if (outfile && !(ofp = fopen(outfile, "w")))
			eprintf("fopen %s:", outfile);

		qsort(linebuf.lines, linebuf.nlines, sizeof *linebuf.lines,
				(int (*)(const void *, const void *))linecmp);

		for (i = 0; i < linebuf.nlines; i++) {
			if (!uflag || i == 0 ||
			    linecmp((const char **)&linebuf.lines[i],
			            (const char **)&linebuf.lines[i - 1])) {
				fputs(linebuf.lines[i], ofp);
			}
		}
	}

	if (fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>") |
	    fshut(stderr, "<stderr>"))
		ret = 2;

	return ret;
}
