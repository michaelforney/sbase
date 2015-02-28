/* See LICENSE file for copyright and license details. */
#include <sys/times.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-p] utility [argument ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	pid_t pid;
	struct tms tms; /* hold user and sys times */
	clock_t rbeg, rend; /* real time */
	long ticks; /* per second */
	int status;

	ARGBEGIN {
	case 'p':
		break;
	default:
		usage();
	} ARGEND;

	if (!*argv)
		usage();

	if ((ticks = sysconf(_SC_CLK_TCK)) <= 0)
		eprintf("sysconf() failed to retrieve clock ticks per second\n");

	if ((rbeg = times(&tms)) < 0)
		eprintf("times() failed to retrieve start times:");

	if (!(pid = fork())) { /* child */
		execvp(*argv, argv);
		enprintf(errno == ENOENT ? 127 : 126, "failed to exec %s:", *argv);
	}
	waitpid(pid, &status, 0);

	if ((rend = times(&tms)) < 0)
		eprintf("times() failed to retrieve end times:");

	fprintf(stderr, "real %f\nuser %f\nsys %f\n",
	        (rend - rbeg)  / (double)ticks,
	        tms.tms_cutime / (double)ticks,
	        tms.tms_cstime / (double)ticks);

	return WIFEXITED(status) ? WEXITSTATUS(status) : 0;
}
