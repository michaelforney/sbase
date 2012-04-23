/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include "arg.h"
#include "util.h"

char *argv0;

void
usage(void)
{
	eprintf("usage: %s [string ...]\n", basename(argv0));
}

int
main(int argc, char *argv[])
{
	int i;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (!argc) {
		for(;;)
			puts("y");
	}

	for (;;) {
		for (i = 0; i < argc; i++) {
			fputs(argv[i], stdout);
			if (argv[i+1] != NULL)
				fputs(" ", stdout);
		}
		fputs("\n", stdout);
	}

	return EXIT_SUCCESS;
}

