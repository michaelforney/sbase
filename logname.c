/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	char *login;

	argv0 = argv[0];
	if (argc != 1)
		eprintf("usage: %s\n", argv0);
	if ((login = getlogin()))
		printf("%s\n", login);
	else
		eprintf("%s: no login name\n", argv0);
	return 0;
}
