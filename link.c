/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	argv0 = argv[0];

	if(argc != 3)
		eprintf("usage: %s target linkname\n", argv0);
	if (link(argv[1], argv[2]) < 0)
		eprintf("link:");
	return 0;
}
