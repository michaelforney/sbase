/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-n] text\n", argv0);
}

int
main(int argc, char *argv[])
{
	bool nflag = false;

	ARGBEGIN {
	case 'n':
		nflag = true;
		break;
	default:
		usage();
	} ARGEND;

	for(; argc > 0; argc--, argv++)
		putword(argv[0]);
	if(!nflag)
		putchar('\n');

	return 0;
}
