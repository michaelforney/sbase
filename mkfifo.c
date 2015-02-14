/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-m mode] name...\n", argv0);
}

int
main(int argc, char *argv[])
{
	mode_t mode = 0;
	mode_t mask;
	int mflag = 0;
	int ret = 0;

	ARGBEGIN {
	case 'm':
		mflag = 1;
		mask = getumask();
		mode = parsemode(EARGF(usage()), mode, mask);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++) {
		if (mkfifo(argv[0], S_IRUSR | S_IWUSR |
		    S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0) {
			weprintf("mkfifo %s:", argv[0]);
			ret = 1;
		}
		if (mflag) {
			if (chmod(argv[0], mode) < 0) {
				weprintf("chmod %s:", argv[0]);
				ret = 1;
			}
		}
	}
	return ret;
}
