/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "util.h"

typedef struct {
	FILE *fp;
	const char *name;
} Fdescr;

static int expand(Fdescr *f, int tabstop);

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
		eprintf("not implemented\n");
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
		for (; argc > 0; argc--) {
			if (!(fp = fopen(*argv, "r"))) {
				eprintf("fopen %s:", *argv);
			}
			dsc.name = *argv;
			dsc.fp = fp;
			expand(&dsc, tabstop);
			fclose(fp);
			argv++;
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

	for (;;) {
		c = in(dsc);
		if (c == WEOF)
			break;

		switch (c) {
		case '\t':
			do {
				col++;
				out(' ');
			} while (col & (tabstop - 1));
			break;
		case '\b':
			if (col)
				col--;
			out(c);
			break;
		case '\n':
			col = 0;
			out(c);
			break;
		default:
			col++;
			out(c);
			break;
		}
	}

	return 0;
}
