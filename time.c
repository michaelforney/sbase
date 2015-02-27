#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/wait.h>

#include "util.h"

void
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
		/* used to specify POSIX output format, but that's the only format we
		 * have for now */
		break;
	default:
		usage();
	} ARGEND;

	if (!*argv)
		usage();

	if ((ticks = sysconf(_SC_CLK_TCK)) < 0)
		eprintf("sysconf() failed to retrieve clock ticks per second:");

	if ((rbeg = times(&tms)) < 0) /* POSIX doesn't say NULL is ok... */
		eprintf("times() failed to retrieve start times:");

	if (!(pid = fork())) { /* child */
		execvp(*argv, argv);
		/* FIXME: check errno for POSIX exit status
		 * 126: found could not be invoked
		 * 127: could not be found
		 * problem is some like EACCESS can mean either...
		 * worth doing manual path search for correct value? also gives more
		 * accurate time as the patch search wouldn't be included in child's
		 * user/sys times... */
		enprintf(127, "failed to exec %s:", *argv);
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
