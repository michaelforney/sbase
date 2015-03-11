/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <grp.h>
#include <unistd.h>

#include "util.h"

static struct stat st;
static int   hflag = 0;
static int   Rflag = 0;
static gid_t gid = -1;
static int   ret = 0;

static void
chgrp(const char *path, int depth, void *data)
{
	char *chownf_name;
	int (*chownf)(const char *, uid_t, gid_t);

	if (recurse_follow == 'P' || (recurse_follow == 'H' && depth) || (hflag && !depth)) {
		chownf_name = "lchown";
		chownf = lchown;
	} else {
		chownf_name = "chown";
		chownf = chown;
	}

	if (chownf(path, st.st_uid, gid) < 0) {
		weprintf("%s %s:", chownf_name, path);
		ret = 1;
	} else if (Rflag) {
		recurse(path, chgrp, depth, NULL);
	}
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
		hflag = 1;
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

	errno = 0;
	if (!(gr = getgrnam(argv[0]))) {
		if (errno)
			eprintf("getgrnam %s:", argv[0]);
		else
			eprintf("getgrnam %s: no such group\n", argv[0]);
	}
	gid = gr->gr_gid;

	for (; *argv; argc--, argv++) {
		if (stat(*argv, &st) < 0) {
			weprintf("stat %s:", *argv);
			ret = 1;
			continue;
		}
		chgrp(*argv, 0, NULL);
	}

	return ret;
}
