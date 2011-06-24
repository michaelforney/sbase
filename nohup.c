/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

enum { Error = 127, Found = 126 };

int
main(int argc, char *argv[])
{
	int fd;

	if(getopt(argc, argv, "") != -1)
		exit(Error);
	if(optind == argc)
		enprintf(Error, "usage: %s command [argument...]\n", argv[0]);

	if(signal(SIGHUP, SIG_IGN) == SIG_ERR)
		enprintf(Error, "signal HUP:");
	if(isatty(STDOUT_FILENO)) {
		if((fd = open("nohup.out", O_APPEND|O_CREAT, S_IRUSR|S_IWUSR)) == -1)
			enprintf(Error, "open nohup.out:");
		if(dup2(fd, STDOUT_FILENO) == -1)
			enprintf(Error, "dup2:");
		close(fd);
	}
	if(isatty(STDERR_FILENO))
		if(dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
			enprintf(Error, "dup2:");

	execvp(argv[optind], &argv[optind]);
	enprintf(errno == ENOENT ? Error : Found, "exec %s:", argv[optind]);
	return Error;
}
