/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-e | -f | -m] [-n] path\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	ssize_t n;
	int nflag = 0, mefflag = 0;
	char buf1[PATH_MAX], buf2[PATH_MAX], arg[PATH_MAX],
	     *p, *slash, *prefix, *lp, *b = buf1;

	ARGBEGIN {
	case 'm':
	case 'e':
	case 'f':
		mefflag = ARGC();
		break;
	case 'n':
		nflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	if (strlen(argv[0]) >= PATH_MAX)
		eprintf("path too long\n");

	switch (mefflag) {
	case 'm':
		slash = strchr(argv[0], '/');
		prefix = (slash == argv[0]) ? "/" : (!slash) ? "./" : "";

		estrlcpy(arg, prefix, sizeof(arg));
		estrlcat(arg, argv[0], sizeof(arg));

		for (lp = "", p = arg + (argv[0][0] == '/'); *p; p++) {
			if (*p != '/')
				continue;
			*p = '\0';
			if (!realpath(arg, b)) {
				*p = '/';
				goto mdone;
			}
			b = (b == buf1) ? buf2 : buf1;
			lp = p;
			*p = '/';
		}
		if (!realpath(arg, b)) {
mdone:
			b = (b == buf1) ? buf2 : buf1;
			estrlcat(b, lp, sizeof(arg));
		}
		break;
	case 'e':
		if (stat(argv[0], &st) < 0)
			eprintf("stat %s:", argv[0]);
		if (!realpath(argv[0], b))
			eprintf("realpath %s:", argv[0]);
		break;
	case 'f':
		if (!realpath(argv[0], b))
			eprintf("realpath %s:", argv[0]);
		break;
	default:
		if ((n = readlink(argv[0], b, PATH_MAX - 1)) < 0)
			eprintf("readlink %s:", argv[0]);
		b[n] = '\0';
	}
	fputs(b, stdout);
	if (!nflag)
		putchar('\n');

	return fshut(stdout, "<stdout>");
}
