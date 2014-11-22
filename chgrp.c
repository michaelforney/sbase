/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

static int gid;
static int status;
static int rflag;
static struct stat st;

static void
usage(void)
{
	eprintf("usage: chgrp [-R] groupname file...\n");
}

static void
chgrp(const char *path)
{
	if (chown(path, st.st_uid, gid) < 0) {
		weprintf("chown %s:", path);
		status = 1;
	}
	if (rflag)
		recurse(path, chgrp);
}

int
main(int argc, char *argv[])
{
	struct group *gr;

	ARGBEGIN {
	case 'R':
		rflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2)
		usage();

	errno = 0;
	gr = getgrnam(argv[0]);
	if (errno)
		eprintf("getgrnam %s:");
	else if (!gr)
		eprintf("getgrnam %s: no such group\n", argv[0]);
	gid = gr->gr_gid;

	while (*++argv) {
		if (stat(*argv, &st) < 0) {
			weprintf("stat %s:", *argv);
			status = 1;
			continue;
		}
		chgrp(*argv);
	}
	return status;
}
