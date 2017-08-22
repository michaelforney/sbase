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
	argv0 = *argv, argv0 ? (argc--, argv++) : (void *)0;

	if (argc)
		usage();
	sync();

	return 0;
}
