/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utf.h"
#include "util.h"

#define NLINES 128
#define NCOLS 800

static Rune **buf;

static int    backspace, notabs, halfline, escape;
static size_t nline, ncol, nchar, nspaces, maxline, bs, pagesize = NLINES;

static void
flush(void)
{
	Rune c;
	size_t i, j;

	for (i = 0; i < maxline; ++i) {
		for (j = 0; j < NCOLS && (c = buf[i][j]); ++j)
			efputrune(&c, stdout, "<stdout>");
		putchar('\n');
	}
	bs = nchar = nline = ncol = 0;
}

static void
forward(size_t n)
{
	size_t lim;

	for (lim = ncol + n; ncol != lim && nchar < NCOLS - 1; ++nchar) {
		switch (buf[nline][nchar]) {
		case '\b':
			--ncol;
			break;
		case '\0':
			buf[nline][nchar] = ' ';
			/* FALLTHROUGH */
		default:
			++ncol;
			break;
		}
	}
}

static void
linefeed(int up, int rcarriage)
{
	size_t oncol = ncol;

	nspaces = 0;
	if (up > 0) {
		if (nline == pagesize - 1) {
			flush();
		}  else {
			if (++nline > maxline)
				maxline = nline;
		}
	} else if (nline > 0) {
		--nline;
	}
	bs = 0;
	if (rcarriage) {
		forward(oncol);
		nchar = ncol = 0;
	}
}

static void
newchar(Rune c)
{
	Rune *cp;

	forward(nspaces);
	nspaces = 0;

	switch (c) {
	case ' ':
		forward(1);
		break;
	case '\r':
		nchar = ncol = 0;
		break;
	case '\t':
		forward(8 - ncol % 8);
		break;
	case '\b':
		if (ncol > 0)
			--ncol;
		if (nchar > 0)
			--nchar;
		bs = 1;
		break;
	default:
		cp = &buf[nline][nchar];
		if (*cp && *cp != ' ' && bs && !backspace && nchar != NCOLS - 3) {
			memmove(cp + 3, cp + 1, (NCOLS - nchar - 2) * sizeof(*cp));
			cp[1] = '\b';
			nchar += 2;
		}
		if (nchar != NCOLS - 1) {
			for (cp = buf[nline]; cp < &buf[nline][nchar]; ++cp) {
				if (*cp == '\0')
					*cp = ' ';
			}
			buf[nline][nchar++] = c;
			++ncol;
		}
		bs = 0;
	}
}

static void
col(void)
{
	Rune r;
	int ret;

	while (efgetrune(&r, stdin, "<stdin>")) {
		switch (r) {
		case '\x1b':
			ret = efgetrune(&r, stdin, "<stdin>");
			switch (r) {
			case '8': /* reverse half-line-feed */
			case '7': /* reverse line-feed */
				linefeed(-1, 0);
				continue;
			case '9':  /* forward half-line-feed */
				if (halfline)
					break;
				linefeed(1, 0);
				continue;
			}
			if (!escape)
				continue;
			newchar('\x1b');
			if (ret)
				newchar(r);
			break;
		case '\v':
			linefeed(-1, 0);
			break;
		case ' ':
			if (!notabs) {
				if (++nspaces != 8)
					continue;
				r = '\t';
				nspaces = 0;
			}
			/* FALLTHROUGH */
		case '\r':
		case '\b':
		case '\t':
			newchar(r);
			break;
		case '\n':
			linefeed(1, 1);
			break;
		default:
			if (!iscntrlrune(r))
				newchar(r);
			break;
		}
	}
}

static void
allocbuf(void)
{
	Rune **bp;

	buf = ereallocarray(NULL, pagesize, sizeof(*buf));
	for (bp = buf; bp < buf + pagesize; ++bp)
		*bp = ereallocarray(NULL, NCOLS, sizeof(**buf));
}

static void
usage(void)
{
	eprintf("usage: %s [-pbfx] [-l num]\n", argv0);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'b':
		backspace = 1;
		break;
	case 'f':
		halfline = 1;
		break;
	case 'l':
		pagesize = estrtonum(EARGF(usage()), 1, MIN(SIZE_MAX, LLONG_MAX));
		break;
	case 'p':
		escape = 1;
		break;
	case 'x':
		notabs = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc)
		usage();

	allocbuf();
	col();
	flush();

	return 0;
}
