/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wctype.h>

#include "utf.h"
#include "util.h"

static int    lflag = 0;
static int    wflag = 0;
static char   cmode = 0;
static size_t tc = 0, tl = 0, tw = 0;

void
output(const char *str, size_t nc, size_t nl, size_t nw)
{
	int noflags = !cmode && !lflag && !wflag;
	int first = 1;

	if (lflag || noflags)
		printf("%*.zu", first ? (first = 0) : 7, nl);
	if (wflag || noflags)
		printf("%*.zu", first ? (first = 0) : 7, nw);
	if (cmode || noflags)
		printf("%*.zu", first ? (first = 0) : 7, nc);
	if (str)
		printf(" %s", str);
	putchar('\n');
}

void
wc(FILE *fp, const char *str)
{
	int word = 0, rlen;
	Rune c;
	size_t nc = 0, nl = 0, nw = 0;

	while ((rlen = readrune(str, fp, &c))) {
		nc += (cmode == 'c') ? rlen :
		      (c != Runeerror) ? 1 : 0;
		if (c == '\n')
			nl++;
		if (!iswspace(c))
			word = 1;
		else if (word) {
			word = 0;
			nw++;
		}
	}
	if (word)
		nw++;
	tc += nc;
	tl += nl;
	tw += nw;
	output(str, nc, nl, nw);
}

static void
usage(void)
{
	eprintf("usage: %s [-c | -m] [-lw] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int i;
	int ret = 0;

	ARGBEGIN {
	case 'c':
		cmode = 'c';
		break;
	case 'm':
		cmode = 'm';
		break;
	case 'l':
		lflag = 1;
		break;
	case 'w':
		wflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		wc(stdin, NULL);
	} else {
		for (i = 0; i < argc; i++) {
			if (!(fp = fopen(argv[i], "r"))) {
				weprintf("fopen %s:", argv[i]);
				ret = 1;
				continue;
			}
			wc(fp, argv[i]);
			fclose(fp);
		}
		if (argc > 1)
			output("total", tc, tl, tw);
	}
	return ret;
}
