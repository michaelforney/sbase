/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static const char *countfmt = "";
static int dflag = 0;
static int uflag = 0;
static int fskip = 0;
static int sskip = 0;

static char *prevline = NULL;
static char *prevoffset = NULL;
static long prevlinecount = 0;

static char *
uniqskip(char *l)
{
	char *lo = l;
	int f = fskip, s = sskip;

	for (; f; --f) {
		while (isblank(*lo))
			lo++;
		while (*lo && !isblank(*lo))
			lo++;
	}
	for (; s && *lo && *lo != '\n'; --s, ++lo);
	return lo;
}

static void
uniqline(FILE *ofp, char *l)
{
	char *loffset = l ? uniqskip(l) : l;

	int linesequel = (!l || !prevline)
		? l == prevline
		: !strcmp(loffset, prevoffset);

	if (linesequel) {
		++prevlinecount;
		return;
	}

	if (prevline) {
		if ((prevlinecount == 1 && !dflag) ||
		    (prevlinecount != 1 && !uflag)) {
			if (*countfmt)
				fprintf(ofp, countfmt, prevlinecount);
			fputs(prevline, ofp);
		}
		free(prevline);
		prevline = prevoffset = NULL;
	}

	if (l) {
		prevline = estrdup(l);
		prevoffset = prevline + (loffset - l);
	}
	prevlinecount = 1;
}

static void
uniq(FILE *fp, FILE *ofp)
{
	char *buf = NULL;
	size_t size = 0;

	while (getline(&buf, &size, fp) != -1)
		uniqline(ofp, buf);
}

static void
uniqfinish(FILE *ofp)
{
	uniqline(ofp, NULL);
}

static void
usage(void)
{
	eprintf("usage: %s [-c] [-d | -u] [-f fields] [-s chars]"
	        " [input [output]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp = stdin, *ofp = stdout;

	ARGBEGIN {
	case 'c':
		countfmt = "%7ld ";
		break;
	case 'd':
		dflag = 1;
		break;
	case 'u':
		uflag = 1;
		break;
	case 'f':
		fskip = estrtonum(EARGF(usage()), 0, INT_MAX);
		break;
	case 's':
		sskip = estrtonum(EARGF(usage()), 0, INT_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 2)
		usage();

	if (argc == 0) {
		uniq(stdin, stdout);
	} else if (argc >= 1) {
		if (strcmp(argv[0], "-") && !(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		if (argc == 2) {
			if (strcmp(argv[1], "-") &&
			    !(ofp = fopen(argv[1], "w")))
				eprintf("fopen %s:", argv[1]);
		}
		uniq(fp, ofp);
		if (fp != stdin)
			fclose(fp);
	} else
		usage();
	uniqfinish(ofp);
	if (ofp != stdout)
		fclose(ofp);

	return 0;
}
