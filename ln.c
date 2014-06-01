/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-fs] target linkname\n", argv0);
}

int
main(int argc, char *argv[])
{
	int (*flink)(const char *, const char *);
	char *fname;
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

	flink = sflag ? symlink : link;
	fname = sflag ? "symlink" : "link";

	if (fflag == true)
		remove(argv[1]);
	if (flink(argv[0], argv[1]) < 0)
		eprintf("%s:", fname);

	return EXIT_SUCCESS;
}
