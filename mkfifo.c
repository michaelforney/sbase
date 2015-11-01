/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-m mode] name ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	mode_t mode = 0, mask;
	int mflag = 0, ret = 0;

	ARGBEGIN {
	case 'm':
		mflag = 1;
		mask = getumask();
		mode = parsemode(EARGF(usage()), mode, mask);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	for (; *argv; argc--, argv++) {
		if (mkfifo(*argv, S_IRUSR | S_IWUSR | S_IRGRP |
		    S_IWGRP | S_IROTH | S_IWOTH) < 0) {
			weprintf("mkfifo %s:", *argv);
			ret = 1;
		} else if (mflag) {
			if (chmod(*argv, mode) < 0) {
				weprintf("chmod %s:", *argv);
				ret = 1;
			}
		}
	}

	return ret;
}
