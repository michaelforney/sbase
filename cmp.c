/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

enum { Same = 0, Diff = 1, Error = 2 };

static void
usage(void)
{
	enprintf(Error, "usage: %s [-l | -s] file1 file2\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp[2];
	size_t i, line = 1, n = 1;
	int lflag = 0, sflag = 0, same = 1, b[2];

	ARGBEGIN {
	case 'l':
		lflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 2 || (lflag && sflag))
		usage();

	if (!strcmp(argv[0], argv[1]))
		return Same;

	for (i = 0; i < 2; i++) {
		if (argv[i][0] == '-' && !argv[i][1])
			argv[i] = "/dev/fd/0";
		fp[i] = fopen(argv[i], "r");
		if (!fp[i]) {
			if (!sflag)
				weprintf("fopen %s:", argv[i]);
			exit(Error);
		}
	}

	for (n = 1; ; n++) {
		b[0] = getc(fp[0]);
		b[1] = getc(fp[1]);

		if (b[0] == b[1]) {
			if (b[0] == EOF)
				break;
			else if (b[0] == '\n')
				line++;
			continue;
		}
		if (b[0] == EOF || b[1] == EOF) {
			if (!sflag)
				fprintf(stderr, "cmp: EOF on %s\n",
				        argv[(b[0] == EOF) ? 0 : 1]);
			exit(Diff);
		}
		if (!lflag) {
			if (!sflag)
				printf("%s %s differ: byte %ld, line %ld\n",
				       argv[0], argv[1], n, line);
			exit(Diff);
		} else {
			printf("%ld %o %o\n", n, b[0], b[1]);
			same = 0;
		}
	}
	return same ? Same : Diff;
}
