/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

extern char **environ;

static void
usage(void)
{
	eprintf("usage: %s [variable...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *var;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc == 1) {
		while (*environ)
			printf("%s\n", *environ++);

		return 0;
	}
	while(*++argv) {
		if ((var = getenv(*argv)))
			printf("%s\n", var);
	}

	return 0;
}
