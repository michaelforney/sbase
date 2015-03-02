/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <grp.h>
#include <unistd.h>

#include "util.h"

static int gid;
static int status;
static int Rflag;
static struct stat st;
static char *chownf_name = "chown";
static int (*chownf)(const char *, uid_t, gid_t) = chown;

static void
chgrp(const char *path, int depth)
{
	if (chownf(path, st.st_uid, gid) < 0) {
		weprintf("%s %s:", chownf_name, path);
		status = 1;
	}
	if (Rflag)
		recurse(path, chgrp, depth);
}

static void
usage(void)
{
	eprintf("usage: chgrp [-h] [-R [-H | -L | -P]] group file ...\n");
}

int
main(int argc, char *argv[])
{
	struct group *gr;

	ARGBEGIN {
	case 'h':
		chownf_name = "lchown";
		chownf = lchown;
		break;
	case 'R':
		Rflag = 1;
		break;
	case 'H':
	case 'L':
	case 'P':
		recurse_follow = ARGC();
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2)
		usage();
	if (recurse_follow == 'P') {
		chownf_name = "lchown";
		chownf = lchown;
	}

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
		chgrp(*argv, 0);
	}
	return status;
}
