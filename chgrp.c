/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <grp.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

static int   hflag = 0;
static gid_t gid = -1;
static int   ret = 0;

static void
chgrp(const char *path, struct stat *st, void *data, struct recursor *r)
{
	char *chownf_name;
	int (*chownf)(const char *, uid_t, gid_t);

	if ((r->maxdepth == 0 && r->follow == 'P') || (r->follow == 'H' && r->depth) || (hflag && !(r->depth))) {
		chownf_name = "lchown";
		chownf = lchown;
	} else {
		chownf_name = "chown";
		chownf = chown;
	}

	if (chownf(path, -1, gid) < 0) {
		weprintf("%s %s:", chownf_name, path);
		ret = 1;
	} else if (S_ISDIR(st->st_mode)) {
		recurse(path, NULL, r);
	}
}

static void
usage(void)
{
	eprintf("usage: %s [-h] [-R [-H | -L | -P]] group file ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct group *gr;
	struct recursor r = { .fn = chgrp, .hist = NULL, .depth = 0, .maxdepth = 1,
	                      .follow = 'P', .flags = 0 };

	ARGBEGIN {
	case 'h':
		hflag = 1;
		break;
	case 'R':
		r.maxdepth = 0;
		break;
	case 'H':
	case 'L':
	case 'P':
		r.follow = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if (argc < 2)
		usage();

	errno = 0;
	if ((gr = getgrnam(argv[0]))) {
		gid = gr->gr_gid;
	} else {
		if (errno)
			eprintf("getgrnam %s:", argv[0]);
		gid = estrtonum(argv[0], 0, UINT_MAX);
	}

	for (argc--, argv++; *argv; argc--, argv++)
		recurse(*argv, NULL, &r);

	return ret || recurse_status;
}
