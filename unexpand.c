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

static void unexpand(Fdescr *dsc);

static bool aflag = false;
static int tabsize = 8;

static void
usage(void)
{
	eprintf("usage: %s [-a] [-t n] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	Fdescr dsc;
	FILE *fp;

	ARGBEGIN {
	case 't':
		tabsize = estrtol(EARGF(usage()), 0);
		if(tabsize <= 0)
			eprintf("unexpand: invalid tabsize\n", argv[0]);
		/* Fallthrough: -t implies -a */
	case 'a':
		aflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		dsc.name = "<stdin>";
		dsc.fp = stdin;
		unexpand(&dsc);
	} else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				continue;
			}
			dsc.name = *argv;
			dsc.fp = fp;
			unexpand(&dsc);
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

static void
unexpandspan(unsigned int n, unsigned int col)
{
	unsigned int off = (col-n) % tabsize;

	if(n + off >= tabsize && n > 1)
		n += off;

	for(; n >= tabsize; n -= tabsize)
		out('\t');
	while(n--)
		out(' ');
}

static void
unexpand(Fdescr *dsc)
{
	unsigned int n = 0, col = 0;
	bool bol = true;
	wint_t c;

	while ((c = in(dsc)) != EOF) {
		switch (c) {
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
			bol = false;
			break;
		case '\n':
			if (bol || aflag)
				unexpandspan(n, col);
			n = col = 0;
			bol = true;
			break;
		default:
			if (bol || aflag)
				unexpandspan(n, col);
			n = 0;
			col++;
			bol = false;
		}
		if ((c != ' ' && c != '\t') || (!aflag && !bol))
			out(c);
	}
	if (n > 0 && (bol || aflag))
		unexpandspan(n, col);
}
