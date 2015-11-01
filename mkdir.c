/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-p] [-m mode] name ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	mode_t mode = 0, mask;
	int pflag = 0, mflag = 0, ret = 0;

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
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
		if (pflag) {
			if (mkdirp(*argv) < 0)
				ret = 1;
		} else if (mkdir(*argv, S_IRWXU | S_IRWXG | S_IRWXO) < 0 &&
		           errno != EEXIST) {
				weprintf("mkdir %s:", *argv);
				ret = 1;
		}
		if (mflag && chmod(*argv, mode) < 0) {
			weprintf("chmod %s:", *argv);
			ret = 1;
		}
	}

	return ret;
}
