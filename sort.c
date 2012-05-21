/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

static int linecmp(const char **, const char **);

static bool rflag = false;
static bool uflag = false;

struct linebuf {
	char **lines;
	long nlines;
	long capacity;
};
#define EMPTY_LINEBUF {NULL, 0, 0,}
static struct linebuf linebuf = EMPTY_LINEBUF;

static void getlines(FILE *, struct linebuf *);

int
main(int argc, char *argv[])
{
	char c;
	long i;
	FILE *fp;

	while((c = getopt(argc, argv, "ru")) != -1)
		switch(c) {
		case 'r':
			rflag = true;
			break;
		case 'u':
			uflag = true;
			break;
		default:
			exit(2);
		}
	if(optind == argc)
		getlines(stdin, &linebuf);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		getlines(fp, &linebuf);
		fclose(fp);
	}
	qsort(linebuf.lines, linebuf.nlines, sizeof *linebuf.lines, (int (*)(const void *, const void *))linecmp);

	for(i = 0; i < linebuf.nlines; i++)
		if(!uflag || i == 0 || strcmp(linebuf.lines[i], linebuf.lines[i-1]) != 0)
			fputs(linebuf.lines[i], stdout);
	return EXIT_SUCCESS;
}

void
getlines(FILE *fp, struct linebuf *b)
{
	char *line = NULL;
	size_t size = 0;

	while(afgets(&line, &size, fp)) {
		if(++b->nlines > b->capacity && !(b->lines = realloc(b->lines, (b->capacity+=512) * sizeof *b->lines)))
			eprintf("realloc:");
		if(!(b->lines[b->nlines-1] = malloc(strlen(line)+1)))
			eprintf("malloc:");
		strcpy(b->lines[b->nlines-1], line);
	}
	free(line);
}

int
linecmp(const char **a, const char **b)
{
	return strcmp(*a, *b) * (rflag ? -1 : +1);
}
