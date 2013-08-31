/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s name...\n", argv0);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for(; argc > 0; argc--, argv++) {
		if(mkfifo(argv[0], S_IRUSR|S_IWUSR|S_IRGRP|\
					S_IWGRP|S_IROTH|S_IWOTH) == -1) {
			eprintf("mkfifo %s:", argv[0]);
		}
	}
	return 0;
}

