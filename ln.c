/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

	if(sflag) {
		flink = symlink;
		fname = "symlink";
	} else {
		flink = link;
		fname = "link";
	}

	if(argc < 2) {
		if((to = strrchr(argv[0], '/')))
			to++;
	} else {
		to = argv[1];
	}

	if (fflag == true)
		remove(argv[1]);
	if (flink(argv[0], to) < 0)
		eprintf("%s:", fname);

	return 0;
}
