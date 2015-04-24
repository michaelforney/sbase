/* See LICENSE file for copyright and license details. */
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	argv0 = argv[0], argc--, argv++;

	if (argc)
		usage();
	sync();

	return 0;
}
