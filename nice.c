/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: nice [-n inc] command [options ...]\n");
}

int
main(int argc, char **argv)
{
	int inc = 10;

	ARGBEGIN {
	case 'n':
		inc = atoi(EARGF(usage()));
		break;
	default:
		usage();
	} ARGEND;

	nice(inc); /* POSIX specifies the nice failure still invokes the command. */

	if(!*argv)
		usage();

	execvp(*argv, argv);
	eprintf("nice: '%s': %s\n", *argv, strerror(errno));

	return EXIT_FAILURE;
}

