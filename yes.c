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
	char **p;

	ARGBEGIN {
	default:
		usage();
	} ARGEND

	for (p = argv; ; p = (*p && *(p + 1)) ? p + 1 : argv) {
		fputs(*p ? *p : "y", stdout);
		putchar((!*p || !*(p + 1)) ? '\n' : ' ');
	}

	return 1; /* not reached */
}
