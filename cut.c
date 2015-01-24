/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"
#include "utf.h"
#include "util.h"

typedef struct Range {
	size_t min, max;
	struct Range *next;
} Range;

static Range *list     = NULL;
static char   mode     = 0;
static char  *delim    = "\t";
static size_t delimlen = 1;
static int    nflag    = 0;
static int    sflag    = 0;

static void
insert(Range *r)
{
	Range *l, *p, *t;

	for (p = NULL, l = list; l; p = l, l = l->next) {
		if (r->max && r->max + 1 < l->min) {
			r->next = l;
			break;
		} else if (!l->max || r->min < l->max + 2) {
			l->min = MIN(r->min, l->min);
			for (p = l, t = l->next; t; p = t, t = t->next)
				if (r->max && r->max + 1 < t->min)
					break;
			l->max = (p->max && r->max) ? MAX(p->max, r->max) : 0;
			l->next = t;
			return;
		}
	}
	if (p)
		p->next = r;
	else
		list = r;
}

static void
parselist(char *str)
{
	char *s;
	size_t n = 1;
	Range *r;

	for (s = str; *s; s++) {
		if (*s == ' ')
			*s = ',';
		if (*s == ',')
			n++;
	}
	r = emalloc(n * sizeof(Range));
	for (s = str; n; n--, s++) {
		r->min = (*s == '-') ? 1 : strtoul(s, &s, 10);
		r->max = (*s == '-') ? strtoul(s + 1, &s, 10) : r->min;
		r->next = NULL;
		if (!r->min || (r->max && r->max < r->min) || (*s && *s != ','))
			eprintf("cut: bad list value\n");
		insert(r++);
	}
}

static size_t
seek(const char *s, size_t pos, size_t *prev, size_t count)
{
	const char *t;
	size_t n = pos - *prev, i;

	if (mode == 'b') {
		if ((t = memchr(s, '\0', n)))
			return t - s;
		if (nflag)
			while (n && !UTF8_POINT(s[n]))
				n--;
		*prev += n;
		return n;
	} else if (mode == 'c') {
		for (n++, t = s; *t; t++)
			if (UTF8_POINT(*t) && !--n)
				break;
	} else {
		for (t = (count < delimlen + 1) ? s : s + delimlen; n && *t; ) {
			if (!strncmp(t, delim, delimlen)) {
				if (!--n && count)
					break;
				t += delimlen;
				continue;
			}
			for (i = 1; !fullrune(t, i); i++);
			t += i;
		}
	}
	*prev = pos;

	return t - s;
}

static void
cut(FILE *fp)
{
	static char *buf = NULL;
	static size_t size = 0;
	char *s;
	size_t i, n, p;
	ssize_t len;
	Range *r;

	while ((len = getline(&buf, &size, fp)) != -1) {
		if (len && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (mode == 'f' && !utfutf(buf, delim)) {
			if (!sflag)
				puts(buf);
			continue;
		}
		for (i = 0, p = 1, s = buf, r = list; r; r = r->next, s += n) {
			s += seek(s, r->min, &p, i);
			i += (mode == 'f') ? delimlen : 1;
			if (!*s)
				break;
			if (!r->max) {
				fputs(s, stdout);
				break;
			}
			n = seek(s, r->max + 1, &p, i);
			i += (mode == 'f') ? delimlen : 1;
			if (fwrite(s, 1, n, stdout) != n)
				eprintf("write error:");
		}
		putchar('\n');
	}
}

static size_t
resolveescapes(char *s, size_t len)
{
	size_t i, off, m;

	for (i = 0; i < len; i++) {
		if (s[i] != '\\')
			continue;
		off = 0;

		switch (s[i + 1]) {
                case '\\': s[i] = '\\'; off++; break;
                case 'a':  s[i] = '\a'; off++; break;
                case 'b':  s[i] = '\b'; off++; break;
                case 'f':  s[i] = '\f'; off++; break;
                case 'n':  s[i] = '\n'; off++; break;
                case 'r':  s[i] = '\r'; off++; break;
                case 't':  s[i] = '\t'; off++; break;
                case 'v':  s[i] = '\v'; off++; break;
		case '\0':
			eprintf("cut: null escape sequence in delimiter\n");
		default:
			eprintf("cut: invalid escape sequence '\\%c' in "
			        "delimiter\n", s[i + 1]);
		}

		for (m = i + 1; m <= len - off; m++)
			s[m] = s[m + off];
		len -= off;
	}

	return len;
}

static void
usage(void)
{
	eprintf("usage: cut -b list [-n] [file ...]\n"
	        "       cut -c list [file ...]\n"
	        "       cut -f list [-d delim] [-s] [file ...]\n");
}

int
main(int argc, char *argv[])
{
	FILE *fp;

	ARGBEGIN {
	case 'b':
	case 'c':
	case 'f':
		mode = ARGC();
		parselist(EARGF(usage()));
		break;
	case 'd':
		delim = EARGF(usage());
		if (!*delim)
			eprintf("cut: empty delimiter\n");
		delimlen = resolveescapes(delim, strlen(delim));
		break;
	case 'n':
		nflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (!mode)
		usage();
	if (!argc)
		cut(stdin);
	else for (; argc--; argv++) {
		if (!strcmp(*argv, "-"))
			cut(stdin);
		else {
			if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				continue;
			}
			cut(fp);
			fclose(fp);
		}
	}
	return 0;
}
