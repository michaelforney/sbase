/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: chroot dir [command [arg...]]\n");
}

int
main(int argc, char *argv[])
{
	char *shell[] = { "/bin/sh", "-i", NULL }, *aux, *p;
	int savederrno;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc < 1)
		usage();

	if((aux = getenv("SHELL")))
		shell[0] = aux;

	if(chroot(argv[0]) == -1)
		eprintf("chroot %s:", argv[0]);

	if(chdir("/") == -1)
		eprintf("chdir:");

	if(argc == 1) {
		p = *shell;
		execvp(*shell, shell);
	} else {
		p = argv[1];
		execvp(argv[1], argv+1);
	}

	savederrno = errno;
	weprintf("execvp %s:", p);
	_exit(savederrno == ENOENT ? 127 : 126);
}
