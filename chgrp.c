/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static int gid;
static int failures = 0;
static int rflag = 0;
static struct stat st;

static void
usage(void)
{
	eprintf("usage: chgrp [-R] groupname file...\n");
}

static void
chgrp(const char *path)
{
	if(chown(path, st.st_uid, gid) == -1) {
		fprintf(stderr, "chgrp: '%s': %s\n", path, strerror(errno));
		failures++;
	}
	if (rflag)
		recurse(path, chgrp);
}

int
main(int argc, char **argv)
{
	struct group *gr;

	ARGBEGIN {
	case 'R':
		rflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if(argc < 2)
		usage();

	errno = 0;
	gr = getgrnam(argv[0]);
	if (errno)
		eprintf("getgrnam %s:");
	else if(!gr)
		eprintf("chgrp: '%s': No such group\n", argv[0]);
	gid = gr->gr_gid;

	while(*++argv) {
		if(stat(*argv, &st) == -1) {
			fprintf(stderr, "chgrp: '%s': %s\n", *argv,
					strerror(errno));
			failures++;
			continue;
		}
		chgrp(*argv);
	}

	return failures;
}

