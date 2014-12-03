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
	size_t d;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	s = strlen(argv[0]) ? basename(argv[0]) : ".";
	if (argc == 2 && *s != '/') {
		d = strlen(s) - strlen(argv[1]);
		if (d >= 0) {
			p = s + d;
			if (strcmp(p, argv[1]) == 0)
				*p = '\0';
		}
	}
	puts(s);
	return 0;
}
