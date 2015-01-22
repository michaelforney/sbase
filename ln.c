/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %1$s [-LPfs] target [linkname]\n"
	        "       %1$s [-LPfs] target... directory\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *fname, *to;
	int sflag = 0;
	int fflag = 0;
	int hasto = 0;
	int dirfd = AT_FDCWD;
	int flags = AT_SYMLINK_FOLLOW;
	struct stat st;

	ARGBEGIN {
	case 'f':
		fflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'L':
		flags |= AT_SYMLINK_FOLLOW;
		break;
	case 'P':
		flags &= ~AT_SYMLINK_FOLLOW;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		usage();

	fname = sflag ? "symlink" : "link";

	if (argc >= 2) {
		if (stat(argv[argc - 1], &st) == 0 && S_ISDIR(st.st_mode)) {
			if ((dirfd = open(argv[argc - 1], O_RDONLY)) < 0)
				eprintf("open:");
		} else if (argc == 2) {
			to = argv[1];
			hasto = 1;
		} else {
			eprintf("destination is not a directory\n");
		}
		argc--;
	}

	for (; argc > 0; argc--, argv++) {
		if (!hasto)
			to = basename(argv[0]);
		if (fflag)
			unlinkat(dirfd, to, 0);
		if ((!sflag ? linkat(AT_FDCWD, argv[0], dirfd, to, flags)
		            : symlinkat(argv[0], dirfd, to)) < 0) {
			eprintf("%s %s <- %s:", fname, argv[0], to);
		}
	}

	return 0;
}
