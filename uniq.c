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

static char *prevline     = NULL;
static char *prevoffset   = NULL;
static long prevlinecount = 0;
static size_t prevlinesiz = 0;

static const char *
uniqskip(const char *l)
{
	const char *lo = l;
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
uniqline(FILE *ofp, const char *l, size_t len)
{
	const char *loffset = l ? uniqskip(l) : l;

	int linesequel = l && prevoffset &&
	                 !strcmp(loffset, prevoffset);

	if (linesequel) {
		++prevlinecount;
		return;
	}

	if (prevoffset) {
		if ((prevlinecount == 1 && !dflag) ||
		    (prevlinecount != 1 && !uflag)) {
			if (*countfmt)
				fprintf(ofp, countfmt, prevlinecount);
			fputs(prevline, ofp);
		}
		prevoffset = NULL;
	}

	if (l) {
		if (!prevline || len >= prevlinesiz) {
			prevlinesiz = len + 1;
			prevline = erealloc(prevline, prevlinesiz);
		}
		memcpy(prevline, l, len);
		prevline[len] = '\0';
		prevoffset = prevline + (loffset - l);
	}
	prevlinecount = 1;
}

static void
uniq(FILE *fp, FILE *ofp)
{
	char *buf = NULL;
	size_t size = 0;
	ssize_t len;

	while ((len = getline(&buf, &size, fp)) > 0)
		uniqline(ofp, buf, (size_t)len);
}

static void
uniqfinish(FILE *ofp)
{
	uniqline(ofp, NULL, 0);
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
	FILE *fp[2] = { stdin, stdout };
	int ret = 0, i;
	char *fname[2] = { "<stdin>", "<stdout>" };

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

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-")) {
			fname[i] = argv[i];
			if (!(fp[i] = fopen(argv[i], (i == 0) ? "r" : "w")))
				eprintf("fopen %s:", argv[i]);
		}
	}

	uniq(fp[0], fp[1]);
	uniqfinish(fp[1]);

	ret |= fshut(fp[0], fname[0]) | fshut(fp[1], fname[1]);

	return ret;
}
