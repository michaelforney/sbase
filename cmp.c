/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

enum { Same = 0, Diff = 1, Error = 2 };

static void
usage(void)
{
	enprintf(Error, "usage: %s [-ls] file1 file2\n", argv0);
}

int
main(int argc, char *argv[])
{
	bool lflag = false;
	bool sflag = false;
	bool same = true;
	int b[2], i;
	long line = 1, n = 1;
	FILE *fp[2];

	ARGBEGIN {
	case 'l':
		lflag = true;
		break;
	case 's':
		sflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if(argc < 2)
		usage();

	for(i = 0; i < 2; i++) {
		if(!(fp[i] = fopen(argv[i], "r")))
			enprintf(Error, "fopen %s:", argv[i]);
	}
	for(n = 1; ((b[0] = getc(fp[0])) != EOF) \
			| ((b[1] = getc(fp[1])) != EOF); n++) {
		if(b[0] == '\n')
			line++;
		if(b[0] == b[1])
			continue;
		for(i = 0; i < 2; i++)
			if(b[i] == EOF)
				enprintf(Diff, "cmp: EOF on %s\n", argv[i]);
		if(!lflag) {
			if(!sflag)
				printf("%s %s differ: char %ld, line %ld\n",
				       argv[0], argv[1], n, line);
			exit(Diff);
		} else {
			printf("%4ld %3o %3o\n", n, b[0], b[1]);
			same = false;
		}
	}
	return same ? Same : Diff;
}

