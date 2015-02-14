/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "util.h"

enum { Error = 127, Found = 126 };

static void
usage(void)
{
	eprintf("usage: %s cmd [arg ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int fd;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		usage();

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
		enprintf(Error, "signal HUP:");

	if (isatty(STDOUT_FILENO)) {
		if ((fd = open("nohup.out", O_APPEND|O_CREAT,
			       S_IRUSR|S_IWUSR)) < 0) {
			enprintf(Error, "open nohup.out:");
		}
		if (dup2(fd, STDOUT_FILENO) < 0)
			enprintf(Error, "dup2:");
		close(fd);
	}
	if (isatty(STDERR_FILENO))
		if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
			enprintf(Error, "dup2:");

	execvp(argv[0], &argv[0]);
	enprintf(errno == ENOENT ? Error : Found, "exec %s:", argv[0]);
	_exit(Error);
	/* unreachable */
	return 0;
}
