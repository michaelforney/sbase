/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void usage(void);

void
usage(void)
{
	eprintf("usage: %s name [suffix]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *s, *p;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	s = basename(argv[0]);
	if (argc == 2) {
		p = strstr(s, argv[1]);
		if (p && p[strlen(p)] == '\0')
			*p = '\0';
	}
	puts(s);
	return 0;
}
