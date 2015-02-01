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
	eprintf("usage: %s path [suffix]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *p;
	size_t off;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	p = strlen(argv[0]) ? basename(argv[0]) : ".";
	if (argc == 2 && *p != '/') {
		if (strlen(argv[1]) < strlen(p)) {
			off = strlen(p) - strlen(argv[1]);
			if (strcmp(&p[off], argv[1]) == 0)
				p[off] = '\0';
		}
	}
	puts(p);
	return 0;
}
