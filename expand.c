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
	int ret = 0;

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
				ret = 1;
				continue;
			}
			expand(argv[0], fp, tabstop);
			fclose(fp);
		}
	}
	return ret;
}

static int
expand(const char *file, FILE *fp, int tabstop)
{
	int col = 0;
	Rune r;
	int bol = 1;

	for (;;) {
		if (!readrune(file, fp, &r))
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
			writerune("<stdout>", stdout, &r);
			break;
		case '\n':
			col = 0;
			bol = 1;
			writerune("<stdout>", stdout, &r);
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
