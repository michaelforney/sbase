/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void uniqline(char *);
static void uniq(FILE *, const char *);
static void uniqfinish(void);

static const char *countfmt = "";
static int dflag = 0;
static int uflag = 0;

static char *prevline = NULL;
static long prevlinecount = 0;

static void
usage(void)
{
	eprintf("usage: %s [-cdiu] [input]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;

	ARGBEGIN {
	case 'i':
		eprintf("not implemented\n");
	case 'c':
		countfmt = "%7ld ";
		break;
	case 'd':
		dflag = 1;
		break;
	case 'u':
		uflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		uniq(stdin, "<stdin>");
	} else if (argc == 1) {
		if (!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		uniq(fp, argv[0]);
		fclose(fp);
	} else
		usage();
	uniqfinish();

	return 0;
}

static void
uniqline(char *l)
{
	int linesequel = (!l || !prevline)
		? l == prevline
		: !strcmp(l, prevline);

	if (linesequel) {
		++prevlinecount;
		return;
	}

	if (prevline) {
		if ((prevlinecount == 1 && !dflag) ||
		    (prevlinecount != 1 && !uflag)) {
			printf(countfmt, prevlinecount);
			fputs(prevline, stdout);
		}
		free(prevline);
		prevline = NULL;
	}

	if (l && !(prevline = strdup(l)))
		eprintf("strdup:");
	prevlinecount = 1;
}

static void
uniq(FILE *fp, const char *str)
{
	char *buf = NULL;
	size_t size = 0;

	while (agetline(&buf, &size, fp) != -1)
		uniqline(buf);
}

static void
uniqfinish(void)
{
	uniqline(NULL);
}
