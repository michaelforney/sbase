/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utf.h"
#include "util.h"

static int      iflag      = 0;
static ssize_t *tablist    = NULL;
static size_t   tablistlen = 0;

static size_t
parselist(const char *s, size_t slen)
{
	size_t i, m, len;
	char *sep;

	if (s[0] == ',' || s[0] == ' ')
		eprintf("expand: tablist can't begin with a ',' or ' '.\n");
	if (s[slen - 1] == ',' || s[slen - 1] == ' ')
		eprintf("expand: tablist can't end with a ',' or ' '.\n");

	len = 1;
	for (i = 0; i < slen; i++) {
		if (s[i] == ',' || s[i] == ' ') {
			if (i > 0 && (s[i - 1] == ',' || s[i - 1] == ' '))
				eprintf("expand: empty field in tablist.\n");
			len++;
		}
	}
	tablist = emalloc((len + 1) * sizeof(ssize_t));

	m = 0;
	for (i = 0; i < slen; i += sep - (s + i) + 1) {
		tablist[m++] = strtol(s + i, &sep, 10);
		if (tablist[m - 1] <= 0)
			eprintf("expand: tab size can't be negative or zero.\n");
		if (*sep && *sep != ',' && *sep != ' ')
			eprintf("expand: invalid number in tablist.\n");
		if (m > 1 && tablist[m - 1] < tablist[m - 2])
			eprintf("expand: tablist must be ascending.\n");
	}

	/* tab length = 1 for the overflowing case later in the matcher */
	tablist[len] = 1;
	return len;
}

static int
expand(const char *file, FILE *fp)
{
	size_t bol = 1, col = 0, i;
	Rune r;

	while (readrune(file, fp, &r)) {
		switch (r) {
		case '\t':
			if (tablistlen == 1)
				i = 0;
			else for (i = 0; i < tablistlen; i++)
				if (col < tablist[i])
					break;
			if (bol || !iflag) {
				do {
					col++;
					putchar(' ');
				} while (col % tablist[i]);
			} else {
				putchar('\t');
				col = tablist[i];
			}
			break;
		case '\b':
			bol = 0;
			if (col)
				col--;
			putchar('\b');
			break;
		case '\n':
			bol = 1;
			col = 0;
			putchar('\n');
			break;
		default:
			col++;
			if (r != ' ')
				bol = 0;
			writerune("<stdout>", stdout, &r);
			break;
		}
	}

	return 0;
}

static void
usage(void)
{
	eprintf("usage: %s [-i] [-t tablist] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *tl = "8";
	int   ret = 0;

	ARGBEGIN {
	case 'i':
		iflag = 1;
		break;
	case 't':
		tl = EARGF(usage());
		if (!*tl)
			eprintf("expand: tablist cannot be empty.\n");
		break;
	default:
		usage();
	} ARGEND;

	tablistlen = parselist(tl, strlen(tl));

	if (argc == 0)
		expand("<stdin>", stdin);
	else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				ret = 1;
				continue;
			}
			expand(argv[0], fp);
			fclose(fp);
		}
	}
	return ret;
}
