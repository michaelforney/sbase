/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static void usage(void);

int
main(int argc, char *argv[])
{
	char *s;
	size_t n;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc < 1)
		usage();

	s = basename(argv[0]);
	if(argc == 2 && argv[1]) {
		n = strlen(s) - strlen(argv[1]);
		if(!strcmp(&s[n], argv[1]))
			s[n] = '\0';
	}
	puts(s);

	return EXIT_SUCCESS;
}

void
usage(void)
{
	eprintf("usage: %s name [suffix]\n", argv0);
}
