/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void uniq_line(char *);
static void uniq(FILE *, const char *);
static void uniq_finish(void);

static const char *countfmt = "";
static bool dflag = false;
static bool uflag = false;

static char *prev_line = NULL;
static long prev_line_count = 0;

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
		dflag = true;
		break;
	case 'u':
		uflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if(argc == 0) {
		uniq(stdin, "<stdin>");
	} else if(argc == 1) {
		if(!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		uniq(fp, argv[0]);
		fclose(fp);
	} else
		usage();
	uniq_finish();

	return EXIT_SUCCESS;
}

void
uniq_line(char *l)
{
	bool lines_equal = ((l == NULL) || (prev_line == NULL))
		? l == prev_line
		: !strcmp(l, prev_line);

	if(lines_equal) {
		++prev_line_count;
		return;
	}

	if(prev_line != NULL) {
		if((prev_line_count == 1 && !dflag) ||
		   (prev_line_count != 1 && !uflag)) {
			printf(countfmt, prev_line_count);
			fputs(prev_line, stdout);
		}
		free(prev_line);
		prev_line = NULL;
	}

	if(l && !(prev_line = strdup(l)))
		eprintf("strdup:");
	prev_line_count = 1;
}

void
uniq(FILE *fp, const char *str)
{
	char *buf = NULL;
	size_t size = 0;

	while(afgets(&buf, &size, fp))
		uniq_line(buf);
}

void
uniq_finish()
{
	uniq_line(NULL);
}

