/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>

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

	if (argc == 0) {
		for (;;)
			puts("y");
	} else {
		for (i = 0; ; i++, i %= argc) {
			printf("%s", argv[i]);
			putchar(i == argc - 1 ? '\n' : ' ');
		}
	}
	return 1; /* should not reach */
}
