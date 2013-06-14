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
	exit(1);
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

	return 0;
}

int
ln(const char *s1, const char *s2)
{
	int (*flink)(const char *, const char *) = sflag ? symlink : link;

	if(flink(s1, s2) == 0)
		return 0;
	if(fflag && errno == EEXIST) {
		if(remove(s2) == -1)
			eprintf("remove %s:", s2);
		return flink(s1, s2);
	}
	return -1;
}

