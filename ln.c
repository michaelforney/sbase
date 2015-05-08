/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-f] [-L | -P | -s] target [name]\n"
	        "       %s [-f] [-L | -P | -s] target ... dir\n", argv0, argv0);
}

int
main(int argc, char *argv[])
{
	char *fname, *to;
	int sflag = 0, fflag = 0, hasto = 0, dirfd = AT_FDCWD, flags = AT_SYMLINK_FOLLOW;
	struct stat st, tost;

	ARGBEGIN {
	case 'f':
		fflag = 1;
		break;
	case 'L':
		flags |= AT_SYMLINK_FOLLOW;
		break;
	case 'P':
		flags &= ~AT_SYMLINK_FOLLOW;
		break;
	case 's':
		sflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();

	fname = sflag ? "symlink" : "link";

	if (argc >= 2) {
		if (!stat(argv[argc - 1], &st) && S_ISDIR(st.st_mode)) {
			if ((dirfd = open(argv[argc - 1], O_RDONLY)) < 0)
				eprintf("open %s:", argv[argc - 1]);
		} else if (argc == 2) {
			to = argv[argc - 1];
			hasto = 1;
		} else {
			eprintf("%s: not a directory\n", argv[argc - 1]);
		}
		argv[argc - 1] = NULL;
		argc--;
	}

	for (; *argv; argc--, argv++) {
		if (!hasto)
			to = basename(*argv);
		if (!sflag) {
			if (stat(*argv, &st) < 0) {
				weprintf("stat %s:", *argv);
				continue;
			} else if (fstatat(dirfd, to, &tost, AT_SYMLINK_NOFOLLOW) < 0) {
				if (errno != ENOENT)
					eprintf("stat %s:", to);
			} else if (st.st_dev == tost.st_dev && st.st_ino == tost.st_ino) {
				weprintf("%s and %s are the same file\n", *argv, *argv);
				continue;
			}
		}
		if (fflag)
			unlinkat(dirfd, to, 0);
		if ((!sflag ? linkat(AT_FDCWD, *argv, dirfd, to, flags)
		            : symlinkat(*argv, dirfd, to)) < 0) {
			weprintf("%s %s <- %s:", fname, *argv, to);
		}
	}

	return 0;
}
