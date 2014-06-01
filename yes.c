/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [string]\n", argv0);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	for (;;)
		puts(argc >= 1 ? argv[0] : "y");
	return EXIT_FAILURE; /* should not reach */
}
