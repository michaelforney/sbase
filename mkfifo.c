/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-m mode] name...\n", argv0);
}

int
main(int argc, char *argv[])
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP |
		S_IWGRP | S_IROTH | S_IWOTH;

	ARGBEGIN {
	case 'm':
		mode = estrtol(EARGF(usage()), 8);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for(; argc > 0; argc--, argv++)
		if(mkfifo(argv[0], mode) == -1)
			eprintf("mkfifo %s:", argv[0]);
	return 0;
}
