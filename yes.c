/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

#define USAGE()  usage("[string]")

int
main(int argc, char *argv[])
{
	char *s = "y";

	ARGBEGIN {
	default:
		USAGE();
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
		USAGE();
	}
	return EXIT_FAILURE; /* should not reach */
}
