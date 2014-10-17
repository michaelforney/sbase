/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
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
	bool sflag = false;
	bool fflag = false;

	ARGBEGIN {
	case 'f':
		fflag = true;
		break;
	case 's':
		sflag = true;
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

	if (fflag == true)
		remove(to);
	if (flink(argv[0], to) < 0)
		eprintf("%s %s <- %s:", fname, argv[0], to);

	return 0;
}
