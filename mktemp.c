/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-d] [template]\n", argv0);
}

static int dflag = 0;

int
main(int argc, char *argv[])
{
	char *template = "tmp.XXXXXXXXXX";
	char *tmpdir = "/tmp";
	char tmppath[PATH_MAX];
	int fd;

	ARGBEGIN {
	case 'd':
		dflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();
	else if (argc == 1)
		template = argv[0];

	snprintf(tmppath, sizeof(tmppath), "%s/%s", tmpdir, template);
	if (dflag) {
		if (!mkdtemp(tmppath))
			eprintf("mkdtemp %s:", tmppath);
	} else {
		if ((fd = mkstemp(tmppath)) < 0)
			eprintf("mkstemp %s:", tmppath);
		close(fd);
	}
	puts(tmppath);
	return EXIT_SUCCESS;
}
