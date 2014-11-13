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

	if (argc != 2)
		usage();

	if (argv[0][0] == '-')
		argv[0] = "/dev/fd/0";
	fp[0] = fopen(argv[0], "r");
	if (!fp[0]) {
		if (!sflag)
			weprintf("fopen %s:", argv[0]);
		exit(Error);
	}

	if (argv[1][0] == '-')
		argv[1] = "/dev/fd/0";
	fp[1] = fopen(argv[1], "r");
	if (!fp[1]) {
		if (!sflag)
			weprintf("fopen %s:", argv[1]);
		exit(Error);
	}

	for (n = 1; ; n++) {
		b[0] = getc(fp[0]);
		b[1] = getc(fp[1]);
		if (b[0] == EOF && b[1] == EOF)
			break;
		if (b[0] == '\n' && b[1] == '\n')
			line++;
		if (b[0] == b[1])
			continue;
		for (i = 0; i < 2; i++) {
			if (b[i] == EOF) {
				if (!sflag)
					fprintf(stderr, "cmp: EOF on %s\n",
					        !argv[i] ? "<stdin>" : argv[i]);
				exit(Diff);
			}
		}
		if (!lflag) {
			if (!sflag)
				printf("%s %s differ: char %ld, line %ld\n",
				       argv[0], !argv[1] ? "<stdin>" : argv[1], n, line);
			exit(Diff);
		} else {
			printf("%4ld %3o %3o\n", n, b[0], b[1]);
			same = false;
		}
	}
	return same ? Same : Diff;
}
