/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static int ln(const char *, const char *);

static bool sflag = false;
static bool fflag = false;

static void
usage(void)
{
	eprintf("usage: %s [-fs] target linkname\n", argv0);
}

int
main(int argc, char *argv[])
{
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

	enmasse(argc, &argv[0], ln);

	return EXIT_SUCCESS;
}

int
ln(const char *s1, const char *s2)
{
	int (*flink)(const char *, const char *) = sflag ? symlink : link;

	if (fflag)
		remove(s2);
	if(flink(s1, s2) == 0)
		return 0;
	return -1;
}
