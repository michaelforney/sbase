/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s seconds\n", argv0);
}

int
main(int argc, char *argv[])
{
	unsigned int seconds;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc < 1)
		usage();

	seconds = estrtol(argv[0], 0);
	while((seconds = sleep(seconds)) > 0)
		;
	return 0;
}
