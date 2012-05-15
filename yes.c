/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void usage(void);

int
main(int argc, char *argv[])
{
	char *s = "y";

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	switch(argc) {
	case 1:
		s = argv[0];
		/* fallthrough */
	case 0:
		for(;;)
			puts(s);
		break;
	default:
		usage();
	}
	return EXIT_FAILURE; /* should not reach */
}

void
usage(void)
{
	eprintf("usage: %s [string]\n", argv0);
}
