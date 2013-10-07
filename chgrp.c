#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include <errno.h>
#include <grp.h>
#include <sys/types.h>
#include "util.h"

static int gid;
static int failures = 0;

static void
usage(void)
{
	eprintf("usage: chgrp [-R] groupname file...\n");
}

static int
chgrp(const char *path, const struct stat *st, int f)
{
	(void)f;

	if(chown(path, st->st_uid, gid) == -1) {
		fprintf(stderr, "chgrp: '%s': %s\n", path, strerror(errno));
		failures++;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	int rflag = 0;
	struct group *gr;
	struct stat st;

	ARGBEGIN {
	case 'R':
		rflag = 1;
		break;
	default:
		usage();
	} ARGEND;
	if(argc<2)
		usage();

	gr = getgrnam(argv[0]);
	if(!gr)
		eprintf("chgrp: '%s': No such group\n", argv[0]);
	gid = gr->gr_gid;

	if(rflag) {
		while(*++argv)
			ftw(*argv, chgrp, FOPEN_MAX);

		return EXIT_SUCCESS;
	}
	while(*++argv) {
		if(stat(*argv, &st) == -1) {
			fprintf(stderr, "chgrp: '%s': %s\n", *argv,
					strerror(errno));
			failures++;
			continue;
		}
		chgrp(*argv, &st, 0);
	}

	return failures;
}

