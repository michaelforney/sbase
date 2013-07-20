#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void usage(void);

int
main(int argc, char **argv)
{
	char *shell[] = {"/bin/sh", "-i", NULL};

	if(getenv("SHELL"))
		shell[0] = getenv("SHELL");

	if(argc < 2)
		usage();

	if(chroot(argv[1]) == -1)
		eprintf("chroot: '%s':", argv[1]);

	if(chdir("/") == -1)
		eprintf("chroot:");

	if(argc == 2) {
		execvp(*shell, shell);
	} else {
		execvp(argv[2], argv+2);
	}

	eprintf("chroot: '%s':", argv[2]);
	return 1;
}

void
usage(void)
{
	eprintf("usage: chroot dir [command [arg...]]\n");
}
