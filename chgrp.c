/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <grp.h>
#include <unistd.h>

#include "util.h"

static int gid;
static int status;
static int rflag;
static struct stat st;
static char *chown_f_name = "chown";
static int (*chown_f)(const char *, uid_t, gid_t) = chown;

static void
chgrp(const char *path)
{
	if (chown_f(path, st.st_uid, gid) < 0) {
		weprintf("%s %s:", chown_f_name, path);
		status = 1;
	}
	if (rflag)
		recurse(path, chgrp);
}

static void
usage(void)
{
	eprintf("usage: chgrp [-hR] groupname file...\n");
}

int
main(int argc, char *argv[])
{
	struct group *gr;

	ARGBEGIN {
	case 'h':
		chown_f_name = "lchown";
		chown_f = lchown;
		break;
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
	if (!gr) {
		if (errno)
			eprintf("getgrnam %s:", argv[0]);
		else
			eprintf("getgrnam %s: no such group\n", argv[0]);
	}
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
