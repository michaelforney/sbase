/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>

#include "utf.h"
#include "util.h"

static int expand(const char *, FILE *, int);

static int iflag = 0;

static void
usage(void)
{
	eprintf("usage: %s [-i] [-t n] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int tabstop = 8;

	ARGBEGIN {
	case 'i':
		iflag = 1;
		break;
	case 't':
		tabstop = estrtol(EARGF(usage()), 0);
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		expand("<stdin>", stdin, tabstop);
	} else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				continue;
			}
			expand(argv[0], fp, tabstop);
			fclose(fp);
		}
	}
	return 0;
}

int
in(const char *file, FILE *fp, Rune *r)
{
	char buf[UTFmax];
	int c, i;

	c = fgetc(fp);
	if (ferror(fp))
		eprintf("%s: read error:", file);
	if (feof(fp))
		return 0;
	if (c < Runeself) {
		*r = (Rune)c;
		return 1;
	}
	buf[0] = c;
	for (i = 1; ;) {
		c = fgetc(fp);
		if (ferror(fp))
			eprintf("%s: read error:", file);
		if (feof(fp))
			return 0;
		buf[i++] = c;
		if (fullrune(buf, i))
			return chartorune(r, buf);
	}
}

static void
out(Rune *r)
{
	char buf[UTFmax];
	int len;

	if ((len = runetochar(buf, r))) {
		fwrite(buf, len, 1, stdout);
		if (ferror(stdout))
			eprintf("stdout: write error:");
	}
}

static int
expand(const char *file, FILE *fp, int tabstop)
{
	int col = 0;
	Rune r;
	int bol = 1;

	for (;;) {
		if (!in(file, fp, &r))
			break;

		switch (r) {
		case '\t':
			if (bol || !iflag) {
				do {
					col++;
					putchar(' ');
				} while (col & (tabstop - 1));
			} else {
				putchar('\t');
				col += tabstop - col % tabstop;
			}
			break;
		case '\b':
			if (col)
				col--;
			bol = 0;
			out(&r);
			break;
		case '\n':
			col = 0;
			bol = 1;
			out(&r);
			break;
		default:
			col++;
			if (r != ' ')
				bol = 0;
			out(&r);
			break;
		}
	}

	return 0;
}
