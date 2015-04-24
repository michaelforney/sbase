/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	char **p;

	argv0 = argv[0], argc--, argv++;

	for (p = argv; ; p = (*p && *(p + 1)) ? p + 1 : argv) {
		fputs(*p ? *p : "y", stdout);
		putchar((!*p || !*(p + 1)) ? '\n' : ' ');
	}

	return 1; /* not reached */
}
