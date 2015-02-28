/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [string ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	size_t i;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	for (i = 0; ; i++, i %= argc ? argc : 1) {
		fputs(argc ? argv[i] : "y", stdout);
		putchar((!argc || i == argc - 1) ? '\n' : ' ');
	}

	return 1; /* not reached */
}
