/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *tty;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	tty = ttyname(STDIN_FILENO);
	puts(tty ? tty : "not a tty");
	return tty ? EXIT_SUCCESS : EXIT_FAILURE;
}
