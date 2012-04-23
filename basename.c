/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "arg.h"
#include "util.h"

char *argv0;

void
usage(void)
{
	eprintf("usage: %s [-ahz] [-s suffix] name [suffix]\n",
			basename(argv0));
}

int
main(int argc, char *argv[])
{
	char *s, *suffix = NULL;
	size_t n, sn;
	bool aflag = false, zflag = false;

	ARGBEGIN {
	case 'a':
		aflag = true;
		break;
	case 's':
		suffix = EARGF(usage());
		break;
	case 'z':
		zflag = true;
		break;
	case 'h':
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	if (!aflag && argc == 2)
		suffix = argv[1];
	if (suffix)
		sn = strlen(suffix);

	for (; argc > 0; argc--, argv++) {
		s = basename(argv[0]);
		if (suffix) {
			n = strlen(s) - sn;
			if (!strcmp(&s[n], suffix))
				s[n] = '\0';
		}
		printf("%s%c", s, (zflag)? '\0' : '\n');

		if (!aflag)
			break;
	}

	return EXIT_SUCCESS;
}

