/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: cut -b list [-n] [file...]\n"
	        "       cut -c list [file...]\n"
	        "       cut -f list [-d delim] [-s] [file...]\n");
}

typedef struct Range {
	size_t min, max;
	struct Range *next;
} Range;

static Range *list = NULL;
static char mode = 0;
static char delim = '\t';
static bool nflag = false;
static bool sflag = false;

static void
insert(Range *r)
{
	Range *l, *p, *t;

	for(p = NULL, l = list; l; p = l, l = l->next) {
		if(r->max && r->max+1 < l->min) {
			r->next = l;
			break;
		} else if(!l->max || r->min < l->max+2) {
			l->min = MIN(r->min, l->min);
			for(p = l, t = l->next; t; p = t, t = t->next)
				if(r->max && r->max+1 < t->min) break;
			l->max = (p->max && r->max) ? MAX(p->max, r->max) : 0;
			l->next = t;
			return;
		}
	}
	if(p) p->next = r; else list = r;
}

static void
parselist(char *str)
{
	char *s;
	size_t n = 1;
	Range *r;

	for(s = str; *s; s++) {
		if(*s == ' ') *s = ',';
		if(*s == ',') n++;
	}
	if(!(r = malloc(n * sizeof(Range))))
		eprintf("malloc:");
	for(s = str; n; n--, s++) {
		r->min = (*s == '-') ? 1 : strtoul(s, &s, 10);
		r->max = (*s == '-') ? strtoul(s + 1, &s, 10) : r->min;
		r->next = NULL;
		if(!r->min || (r->max && r->max < r->min) || (*s && *s != ','))
			eprintf("cut: bad list value\n");
		insert(r++);
	}
}

static size_t
seek(const char *s, size_t pos, size_t *prev, size_t count)
{
	const char *t;
	size_t n = pos - *prev;

	if(mode == 'b') {
		if((t = memchr(s, 0, n)))
			return t - s;
		if(nflag)
			while(n && !UTF8_POINT(s[n])) n--;
		*prev += n;
		return n;
	} else if(mode == 'c') {
		for(n++, t = s; *t; t++)
			if(UTF8_POINT(*t) && !--n) break;
	} else {
		for(t = (count < 2) ? s : s+1; n && *t; t++)
			if(*t == delim && !--n && count) break;
	}
	*prev = pos;
	return t - s;
}

static void
cut(FILE *fp)
{
	char *buf = NULL;
	size_t size = 0;
	char *s;
	size_t i, n, p;
	Range *r;

	while(afgets(&buf, &size, fp)) {
		if(buf[i = strlen(buf)-1] == '\n')
			buf[i] = 0;
		if(mode == 'f' && !strchr(buf, delim)) {
			if(!sflag)
				puts(buf);
			continue;
		}
		for(i = 0, p = 1, s = buf, r = list; r; r = r->next, s += n) {
			s += seek(s, r->min, &p, i++);
			if(!*s) break;
			if(!r->max) {
				fputs(s, stdout);
				break;
			}
			n = seek(s, r->max + 1, &p, i++);
			if(fwrite(s, 1, n, stdout) != n)
				eprintf("write error:");
		}
		putchar('\n');
	}
	free(buf);
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
		parselist(ARGF());
		break;
	case 'd':
		delim = *ARGF();
		break;
	case 'n':
		nflag = true;
		break;
	case 's':
		sflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if(!mode)
		usage();
	if(!argc)
		cut(stdin);
	else for(; argc--; argv++) {
		if(!(fp = strcmp(*argv, "-") ? fopen(*argv, "r") : stdin)) {
			weprintf("fopen %s:", *argv);
			continue;
		}
		cut(fp);
		fclose(fp);
	}
	return EXIT_SUCCESS;
}
