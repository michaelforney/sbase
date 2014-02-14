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
	long sz;
	char *host;

	sz = sysconf(_SC_HOST_NAME_MAX);
	if (sz < 0)
		sz = 255;

	host = malloc(sz + 1);
	if (!host)
		eprintf("malloc:");

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1) {
		if (gethostname(host, sz + 1) < 0)
			eprintf("gethostname:");
		puts(host);
	} else {
		if (sethostname(argv[0], sz + 1) < 0)
			eprintf("sethostname:");
	}

	free(host);

	return EXIT_SUCCESS;
}
