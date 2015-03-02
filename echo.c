/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-n] [string ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int nflag = 0;

	ARGBEGIN {
	case 'n':
		nflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	for (; *argv; argc--, argv++)
		putword(*argv);
	if (!nflag)
		putchar('\n');

	return 0;
}
