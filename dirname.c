/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	if(getopt(argc, argv, "") != -1)
		exit(EXIT_FAILURE);
	if(optind != argc-1)
		eprintf("usage: %s string\n", argv[0]);

	puts(dirname(argv[optind]));
	return EXIT_SUCCESS;
}
