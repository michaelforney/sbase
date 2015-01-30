/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "utf.h"
#include "util.h"

static void unexpand(const char *, FILE *);

static int aflag = 0;
static int tabsize = 8;

static void
usage(void)
{
	eprintf("usage: %s [-a] [-t n] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;

	ARGBEGIN {
	case 't':
		tabsize = estrtonum(EARGF(usage()), 0, INT_MAX);
		if (tabsize <= 0)
			eprintf("unexpand: invalid tabsize\n");
		/* Fallthrough: -t implies -a */
	case 'a':
		aflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		unexpand("<stdin>", stdin);
	} else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				ret = 1;
				continue;
			}
			unexpand(argv[0], fp);
			fclose(fp);
		}
	}
	return ret;
}

static void
unexpandspan(unsigned int n, unsigned int col)
{
	unsigned int off = (col-n) % tabsize;
	Rune r;

	if (n + off >= tabsize && n > 1)
		n += off;

	r = '\t';
	for (; n >= tabsize; n -= tabsize)
		writerune("<stdout>", stdout, &r);
	r = ' ';
	while (n--)
		writerune("<stdout>", stdout, &r);
}

static void
unexpand(const char *file, FILE *fp)
{
	unsigned int n = 0, col = 0;
	Rune r;
	int bol = 1;

	while (1) {
		if (!readrune(file, fp, &r))
			break;

		switch (r) {
		case ' ':
			if (bol || aflag)
				n++;
			col++;
			break;
		case '\t':
			if (bol || aflag)
				n += tabsize - col % tabsize;
			col += tabsize - col % tabsize;
			break;
		case '\b':
			if (bol || aflag)
				unexpandspan(n, col);
			col -= (col > 0);
			n = 0;
			bol = 0;
			break;
		case '\n':
			if (bol || aflag)
				unexpandspan(n, col);
			n = col = 0;
			bol = 1;
			break;
		default:
			if (bol || aflag)
				unexpandspan(n, col);
			n = 0;
			col++;
			bol = 0;
		}
		if ((r != ' ' && r != '\t') || (!aflag && !bol))
			writerune("<stdout>", stdout, &r);
	}
	if (n > 0 && (bol || aflag))
		unexpandspan(n, col);
}
