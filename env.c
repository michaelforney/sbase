/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

extern char **environ;

static void
usage(void)
{
	eprintf("usage: env [-i] [-u variable] ... [variable=value] ... [cmd [arg ...]]\n");
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	case 'i':
		if(environ)
			*environ = NULL;
		break;
	case 'u':
		unsetenv(EARGF(usage()));
		break;
	default:
		usage();
	} ARGEND;

	for (; *argv && strchr(*argv, '='); argv++)
		putenv(*argv);

	if (*argv) {
		execvp(*argv, argv);
		enprintf(127 - (errno != EEXIST), "env: '%s':", *argv);
	}

	while (environ && *environ)
		printf("%s\n", *environ++);

	return 0;
}
