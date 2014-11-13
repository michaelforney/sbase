/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

struct keydef {
	int start_column;
	int end_column;
	int start_char;
	int end_char;
	int flags;
};

enum {
	MOD_N      = 1 << 1,
	MOD_STARTB = 1 << 2,
	MOD_ENDB   = 1 << 3,
	MOD_R      = 1 << 4,
};

struct kdlist {
	struct keydef keydef;
	struct kdlist *next;
};

static struct kdlist *head = NULL;
static struct kdlist *tail = NULL;

static void addkeydef(char *, int);
static void freelist(void);
static int linecmp(const char **, const char **);
static char *skipblank(char *);
static int parse_flags(char **, int *, int);
static int parse_keydef(struct keydef *, char *, int);
static char *nextcol(char *);
static char *columns(char *, const struct keydef *);

static int uflag = 0;
static char *fieldsep = NULL;

static void
usage(void)
{
	enprintf(2, "usage: %s [-bnru] [-t delim] [-k def]... [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	long i;
	FILE *fp;
	struct linebuf linebuf = EMPTY_LINEBUF;
	int global_flags = 0;

	ARGBEGIN {
	case 'b':
		global_flags |= MOD_STARTB | MOD_ENDB;
		break;
	case 'k':
		addkeydef(EARGF(usage()), global_flags);
		break;
	case 'n':
		global_flags |= MOD_N;
		break;
	case 'r':
		global_flags |= MOD_R;
		break;
	case 't':
		fieldsep = EARGF(usage());
		if (strlen(fieldsep) != 1)
			usage();
		break;
	case 'u':
		uflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (!head && global_flags)
		addkeydef("1", global_flags);
	addkeydef("1", global_flags & MOD_R);

	if (argc == 0) {
		getlines(stdin, &linebuf);
	} else for (; argc > 0; argc--, argv++) {
		if (!(fp = fopen(argv[0], "r"))) {
			enprintf(2, "fopen %s:", argv[0]);
			continue;
		}
		getlines(fp, &linebuf);
		fclose(fp);
	}
	qsort(linebuf.lines, linebuf.nlines, sizeof *linebuf.lines,
			(int (*)(const void *, const void *))linecmp);

	for (i = 0; i < linebuf.nlines; i++) {
		if (!uflag || i == 0 || linecmp((const char **)&linebuf.lines[i],
					(const char **)&linebuf.lines[i-1])) {
			fputs(linebuf.lines[i], stdout);
		}
	}

	freelist();
	return 0;
}

static void
addkeydef(char *def, int flags)
{
	struct kdlist *node;

	node = malloc(sizeof(*node));
	if (!node)
		enprintf(2, "malloc:");
	if (!head)
		head = node;
	if (parse_keydef(&node->keydef, def, flags))
		enprintf(2, "faulty key definition\n");
	if (tail)
		tail->next = node;
	node->next = NULL;
	tail = node;
}

static void
freelist(void)
{
	struct kdlist *node;
	struct kdlist *tmp;

	for (node = head; node; node = tmp) {
		tmp = node->next;
		free(node);
	}
}

static int
linecmp(const char **a, const char **b)
{
	char *s1, *s2;
	int res = 0;
	struct kdlist *node;

	for (node = head; node && res == 0; node = node->next) {
		s1 = columns((char *)*a, &node->keydef);
		s2 = columns((char *)*b, &node->keydef);

		/* if -u is given, don't use default key definition
		 * unless it is the only one */
		if (uflag && node == tail && head != tail)
			res = 0;
		else if (node->keydef.flags & MOD_N)
			res = strtol(s1, 0, 10) - strtol(s2, 0, 10);
		else
			res = strcmp(s1, s2);

		if (node->keydef.flags & MOD_R)
			res = -res;

		free(s1);
		free(s2);
	}
	return res;
}

static int
parse_flags(char **s, int *flags, int bflag)
{
	while (isalpha((int)**s))
		switch (*((*s)++)) {
		case 'b':
			*flags |= bflag;
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
	return 0;
}

static int
parse_keydef(struct keydef *kd, char *s, int flags)
{
	char *rest = s;

	kd->start_column = 1;
	kd->start_char = 1;
	/* 0 means end of line */
	kd->end_column = 0;
	kd->end_char = 0;
	kd->flags = flags;

	kd->start_column = strtol(rest, &rest, 10);
	if (kd->start_column < 1)
		return -1;
	if (*rest == '.')
		kd->start_char = strtol(rest+1, &rest, 10);
	if (kd->start_char < 1)
		return -1;
	if (parse_flags(&rest, &kd->flags, MOD_STARTB) == -1)
		return -1;
	if (*rest == ',') {
		kd->end_column = strtol(rest+1, &rest, 10);
		if (kd->end_column && kd->end_column < kd->start_column)
			return -1;
		if (*rest == '.') {
			kd->end_char = strtol(rest+1, &rest, 10);
			if (kd->end_char < 1)
				return -1;
		}
		if (parse_flags(&rest, &kd->flags, MOD_ENDB) == -1)
			return -1;
	}
	if (*rest != '\0')
		return -1;
	return 0;
}

static char *
skipblank(char *s)
{
	while(*s && isblank(*s))
		s++;
	return s;
}

static char *
nextcol(char *s)
{
	if (!fieldsep) {
		s = skipblank(s);
		while(*s && !isblank(*s))
			s++;
	} else {
		if (!strchr(s, *fieldsep))
			s = strchr(s, '\0');
		else
			s = strchr(s, *fieldsep) + 1;
	}
	return s;
}

static char *
columns(char *line, const struct keydef *kd)
{
	char *start, *end;
	char *res;
	int i;

	for (i = 1, start = line; i < kd->start_column; i++)
		start = nextcol(start);
	if (kd->flags & MOD_STARTB)
		start = skipblank(start);
	start += MIN(kd->start_char, nextcol(start) - start) - 1;

	if (kd->end_column) {
		for (i = 1, end = line; i < kd->end_column; i++)
			end = nextcol(end);
		if (kd->flags & MOD_ENDB)
			end = skipblank(end);
		if (kd->end_char)
			end += MIN(kd->end_char, nextcol(end) - end);
		else
			end = nextcol(end);
	} else {
		if (!(end = strchr(line, '\n')))
			end = strchr(line, '\0');
	}

	if (!(res = strndup(start, end - start)))
		enprintf(2, "strndup:");
	return res;
}
