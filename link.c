/* See LICENSE file for copyright and license details. */
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s target name\n", argv0);
}

int
main(int argc, char *argv[])
{
	argv0 = argv[0], argc--, argv++;

	if (argc != 2)
		usage();

	if (link(argv[0], argv[1]) < 0)
		eprintf("link:");

	return 0;
}
