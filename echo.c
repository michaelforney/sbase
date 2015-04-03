/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	int nflag = 0;

	if (*++argv && !strcmp(*argv, "-n")) {
		nflag = 1;
		argc--, argv++;
	}

	for (; *argv; argc--, argv++)
		putword(*argv);
	if (!nflag)
		putchar('\n');

	return 0;
}
