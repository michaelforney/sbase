/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/types.h>
#ifndef makedev
#include <sys/sysmacros.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-m mode] name type major minor\n", argv0);
}

int
main(int argc, char *argv[])
{
	mode_t type, mode = 0666;
	dev_t dev;

	ARGBEGIN {
	case 'm':
		mode = parsemode(EARGF(usage()), mode, umask(0));
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 4)
		usage();

	if (strlen(argv[1]) != 1 || !strchr("ucb", argv[1][0]))
		eprintf("invalid type '%s'\n", argv[1]);
	type = (argv[1][0] == 'b') ? S_IFBLK : S_IFCHR;

	dev = makedev(estrtonum(argv[2], 0, LLONG_MAX), estrtonum(argv[3], 0, LLONG_MAX));

	if (mknod(argv[0], type|mode, dev) == -1)
		eprintf("mknod %s:", argv[0]);
	return 0;
}
