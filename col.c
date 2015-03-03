/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"

#define NLINES 256
#define NCOLS 800

char **buff;

int obackspace, onotabs, ohalfline, oescape;
unsigned nline, ncol, nchar, nspaces, maxline, bs;
size_t pagsize = NLINES;

static void
usage(void)
{
	enprintf(2, "usage: %s [-p][-l num][-b][-f][-x]\n", argv0);
}

static void
flush(void)
{
	int c;
	unsigned i, j;

	for (i = 0; i < maxline; ++i) {
		for (j = 0; j < NCOLS && (c = buff[i][j]) != '\0'; ++j)
			putchar(c);
		putchar('\n');
	}
	bs = nchar = nline = ncol = 0;
}

static void
forward(unsigned n)
{
	unsigned lim;

	for (lim = ncol + n; ncol != lim && nchar < NCOLS-1; ++nchar) {
		switch (buff[nline][nchar]) {
		case '\b':
			--ncol;
			break;
		case '\0':
			buff[nline][nchar] = ' ';
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
	unsigned oncol = ncol;

	nspaces = 0;
	if (up > 0) {
		if (nline == pagsize-1) {
			flush();
		}  else {
			if (++nline > maxline)
				maxline = nline;
		}
	} else {
		if (nline > 0)
			--nline;
	}
	bs = 0;
	if (rcarriage) {
		forward(oncol);
		 nchar = ncol = 0;
	}
}

static void
newchar(int c)
{
	char *cp;

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
		forward(8 - ncol%8);
		break;
	case '\b':
		if (ncol > 0)
			--ncol;
		if (nchar > 0)
			--nchar;
		bs = 1;
		break;
	default:
		cp = &buff[nline][nchar];
		if (*cp != '\0' && *cp != ' ' && bs && !obackspace) {
			if (nchar != NCOLS-3) {
				memmove(cp + 3, cp + 1, NCOLS - nchar - 2);
				cp[1] = '\b';
				nchar += 2;
			}
		}
		if (nchar != NCOLS-1) {
			for (cp = buff[nline]; cp < &buff[nline][nchar]; ++cp) {
				if (*cp == '\0')
					*cp = ' ';
			}
			buff[nline][nchar++] = c;
			++ncol;
		}
		bs = 0;
	}
}

static void
col(void)
{
	int c;

	while ((c = getchar()) != EOF) {
		switch (c) {
		case '\x1b':
			switch (c = getchar()) {
			case '8': /* reverse half-line-feed */
			case '7': /* reverse line-feed */
				linefeed(-1, 0);
				continue;
			case '9':  /* forward half-line-feed */
				if (ohalfline)
					break;
				linefeed(1, 0);
				continue;
			}
			if (!oescape)
				continue;
			newchar('\x1b');
			if (c != EOF)
				newchar(c);			
			break;
		case '\v':
			linefeed(-1, 0);
			break;
		case ' ':
			if (!onotabs) {
				if (++nspaces != 8)
					continue;
				c = '\t';
				nspaces = 0;
			}
			/* FALLTHROUGH */
		case '\r':
		case '\b':
		case '\t':
			newchar(c);
			break;
		case '\n':
			linefeed(1, 1);
			break;
		default:
			if (!iscntrl(c))
				newchar(c);
			break;
		}
	}
}

static void
allocbuf(void)
{
	char **bp;

	buff = emalloc(sizeof(*buff) * pagsize);
	for (bp = buff; bp < &buff[pagsize]; ++bp)
		*bp = emalloc(NCOLS);	
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'b':
		obackspace = 1;
		break;
	case 'f':
		ohalfline = 1;
		break;
	case 'l':
		pagsize = estrtonum(EARGF(usage()), 0, SIZE_MAX);
		break;
	case 'p':
		oescape = 1;
		break;
	case 'x':
		onotabs = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 0)
		usage();

	allocbuf();
	col();
	flush();

	if (ferror(stdin))
		enprintf(1, "error reading input");
	if (ferror(stdout))
		enprintf(2, "error writing output");

	return 0;
}
