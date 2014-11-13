/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: rmdir dir...\n");
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++)
		if (rmdir(argv[0]) == -1)
			fprintf(stderr, "rmdir: '%s': %s\n",
				argv[0], strerror(errno));

	return 0;
}
