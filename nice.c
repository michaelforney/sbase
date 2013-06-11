/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void eusage(void);

int
main(int argc, char **argv)
{
	long val = 10;

	ARGBEGIN {
	case 'n':
		val = estrtol(EARGF(eusage()), 10);
		break;
	default:
		eusage();
		break;
	} ARGEND;

	if(argc == 0)
		eusage();

	errno = 0;
	nice((int)MAX(INT_MIN, MIN(val, INT_MAX)));
	if(errno != 0)
		perror("can't adjust niceness");

	/* POSIX specifies the nice failure still invokes the command */
	execvp(argv[0], argv);
	/* reached only on failure */
	perror(argv[0]);
	return (errno == ENOENT)? 127 : 126;
}

static void
eusage(void)
{
	eprintf("usage: nice [-n inc] command [options ...]\n");
}
