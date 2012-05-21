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

int
main(int argc, char *argv[])
{
	int c;
	FILE *fp;

	while((c = getopt(argc, argv, "cdu")) != -1)
		switch(c) {
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
			exit(2);
		}

	if(optind == argc)
		uniq(stdin, "<stdin>");
	else if(optind == argc - 1) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		uniq(fp, argv[optind]);
		fclose(fp);
	} else
		enprintf(2, "too many arguments\n");
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
