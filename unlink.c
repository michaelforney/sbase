/* See LICENSE file for copyright and license details. */
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s file\n", argv0);
}

int
main(int argc, char *argv[])
{
	argv0 = *argv, argv0 ? (argc--, argv++) : (void *)0;

	if (argc != 1)
		usage();

	if (unlink(argv[0]) < 0)
		eprintf("unlink: '%s':", argv[0]);

	return 0;
}
