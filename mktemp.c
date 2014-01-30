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
	eprintf("usage: %s [-dq] [template]\n", argv0);
}

static int dflag = 0;
static int qflag = 0;

int
main(int argc, char *argv[])
{
	char *template = "tmp.XXXXXXXXXX";
	char *tmpdir = "/tmp", *p;
	char tmppath[PATH_MAX];
	int fd;

	ARGBEGIN {
	case 'd':
		dflag = 1;
		break;
	case 'q':
		qflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();
	else if (argc == 1)
		template = argv[0];

	if ((p = getenv("TMPDIR")))
		tmpdir = p;

	if (snprintf(tmppath, sizeof(tmppath), "%s/%s", tmpdir, template) >= sizeof(tmppath))
		eprintf("path too long\n");
	if (dflag) {
		if (!mkdtemp(tmppath)) {
			if (!qflag)
				eprintf("mkdtemp %s:", tmppath);
			exit(EXIT_FAILURE);
		}
	} else {
		if ((fd = mkstemp(tmppath)) < 0) {
			if (!qflag)
				eprintf("mkstemp %s:", tmppath);
			exit(EXIT_FAILURE);
		}
		close(fd);
	}
	puts(tmppath);
	return EXIT_SUCCESS;
}
