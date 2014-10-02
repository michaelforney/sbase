/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "util.h"

typedef struct {
	FILE *fp;
	const char *name;
} Fdescr;

static int expand(Fdescr *f, int tabstop);

static bool iflag = false;

static void
usage(void)
{
	eprintf("usage: %s [-i] [-t n] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	Fdescr dsc;
	FILE *fp;
	int tabstop = 8;

	ARGBEGIN {
	case 'i':
		iflag = true;
		break;
	case 't':
		tabstop = estrtol(EARGF(usage()), 0);
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		dsc.name = "<stdin>";
		dsc.fp = stdin;
		expand(&dsc, tabstop);
	} else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				continue;
			}
			dsc.name = *argv;
			dsc.fp = fp;
			expand(&dsc, tabstop);
			fclose(fp);
		}
	}
	return 0;
}

static wint_t
in(Fdescr *f)
{
	wint_t c = fgetwc(f->fp);

	if (c == WEOF && ferror(f->fp))
		eprintf("'%s' read error:", f->name);

	return c;
}

static void
out(wint_t c)
{
	putwchar(c);
	if (ferror(stdout))
		eprintf("write error:");
}

static int
expand(Fdescr *dsc, int tabstop)
{
	int col = 0;
	wint_t c;
	bool bol = true;

	for (;;) {
		c = in(dsc);
		if (c == WEOF)
			break;

		switch (c) {
		case '\t':
			if (bol || !iflag) {
				do {
					col++;
					out(' ');
				} while (col & (tabstop - 1));
			} else {
				out('\t');
				col += tabstop - col % tabstop;
			}
			break;
		case '\b':
			if (col)
				col--;
			bol = false;
			out(c);
			break;
		case '\n':
			col = 0;
			bol = true;
			out(c);
			break;
		default:
			col++;
			if (c != ' ')
				bol = false;
			out(c);
			break;
		}
	}

	return 0;
}
