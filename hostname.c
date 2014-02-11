/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [name]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char host[HOST_NAME_MAX + 1];

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1) {
		if (gethostname(host, sizeof(host)) < 0)
			eprintf("gethostname:");
		puts(host);
	} else {
		if (sethostname(argv[0], strlen(argv[0])) < 0)
			eprintf("sethostname:");
	}

	return EXIT_SUCCESS;
}
