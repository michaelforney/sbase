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
	int ret = 0;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		while (*environ)
			printf("%s\n", *environ++);
	} else {
		while (*argv) {
			if ((var = getenv(*argv)))
				printf("%s\n", var);
			else
				ret = 1;
			argv++;
		}
	}
	return ret;
}
