/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include "util.h"

static void usage(void);

int
main(int argc, char **argv)
{
	long val = 10;
	int savederrno;

	ARGBEGIN {
	case 'n':
		val = estrtol(EARGF(usage()), 10);
		break;
	default:
		usage();
		break;
	} ARGEND;

	if(argc == 0)
		usage();

	errno = 0;
	val += getpriority(PRIO_PROCESS, 0);
	if (errno != 0)
		weprintf("getpriority:");
	val = MAX(-NZERO, MIN(val, NZERO - 1));
	if (setpriority(PRIO_PROCESS, 0, val) != 0)
		weprintf("setpriority:");

	/* POSIX specifies the nice failure still invokes the command */
	execvp(argv[0], argv);
	savederrno = errno;
	weprintf("execvp %s:", argv[0]);
	return (savederrno == ENOENT)? 127 : 126;
}

static void
usage(void)
{
	eprintf("usage: nice [-n inc] command [options ...]\n");
}
