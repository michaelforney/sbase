/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

static int
mkdirp(char *path)
{
	char *p = path;

	do {
		if (*p && (p = strchr(&p[1], '/')))
			*p = '\0';
		if (mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO) < 0 && errno != EEXIST) {
			weprintf("mkdir %s:", path);
			return -1;
		}
		if (p)
			*p = '/';
	} while (p);
	return 0;
}

static void
usage(void)
{
	eprintf("usage: %s [-pm] directory...\n", argv0);
}

int
main(int argc, char *argv[])
{
	int pflag = 0;
	int mflag = 0;
	int mode;
	int r = 0;

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	case 'm':
		mflag = 1;
		mode = estrtol(EARGF(usage()), 8);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++) {
		if (pflag) {
			if (mkdirp(argv[0]) < 0)
				r = 1;
		} else if (mkdir(argv[0], S_IRWXU|S_IRWXG|S_IRWXO) < 0) {
			weprintf("mkdir %s:", argv[0]);
			r = 1;
		}
		if (mflag) {
			if (chmod(argv[0], mode) < 0) {
				weprintf("chmod %s:", argv[0]);
				r = 1;
			}
		}
	}

	return 0;
}
