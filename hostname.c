/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [name]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char buf[BUFSIZ];

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1) {
		if (gethostname(buf, sizeof(buf)) < 0)
			eprintf("gethostname:");
		puts(buf);
	} else {
		if (sethostname(argv[0], strlen(argv[0])) < 0)
			eprintf("sethostname:");
	}

	return 0;
}
