/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-dq] [template]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int dflag = 0, qflag = 0, fd;
	char *template = "tmp.XXXXXXXXXX",
	     *tmpdir = "/tmp", *p,
	      path[PATH_MAX], tmp[PATH_MAX];

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

	if (strlcpy(tmp, template, sizeof(tmp)) >= sizeof(tmp))
		eprintf("path too long\n");
	p = dirname(tmp);
	if (p[0] != '.') {
		if (strlcpy(path, template, sizeof(path)) >= sizeof(path))
			eprintf("path too long\n");
	} else {
		if (strlcpy(path, tmpdir, sizeof(path)) >= sizeof(path))
			eprintf("path too long\n");
		if (strlcat(path, "/", sizeof(path)) >= sizeof(path))
			eprintf("path too long\n");
		if (strlcat(path, template, sizeof(path)) >= sizeof(path))
			eprintf("path too long\n");
	}

	if (dflag) {
		if (!mkdtemp(path)) {
			if (!qflag)
				eprintf("mkdtemp %s:", path);
			return 1;
		}
	} else {
		if ((fd = mkstemp(path)) < 0) {
			if (!qflag)
				eprintf("mkstemp %s:", path);
			return 1;
		}
		close(fd);
	}
	puts(path);

	return 0;
}
