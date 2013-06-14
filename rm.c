/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fs.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-fr] FILE...\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'f':
		rm_fflag = true;
		break;
	case 'r':
		rm_rflag = true;
		break;
	default:
		usage();
	} ARGEND;
	for(; argc > 0; argc--, argv++)
		rm(argv[0]);

	return 0;
}

