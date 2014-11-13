/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-fs] target [linkname]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int (*flink)(const char *, const char *);
	char *fname, *to;
	int sflag = 0;
	int fflag = 0;

	ARGBEGIN {
	case 'f':
		fflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0 || argc > 2)
		usage();

	if (sflag) {
		flink = symlink;
		fname = "symlink";
	} else {
		flink = link;
		fname = "link";
	}

	to = argc < 2 ? basename(argv[0]) : argv[1];

	if (fflag)
		remove(to);
	if (flink(argv[0], to) < 0)
		eprintf("%s %s <- %s:", fname, argv[0], to);

	return 0;
}
