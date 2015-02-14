/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
	eprintf("usage: %s [-p] [-m mode] directory ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	mode_t mode = 0;
	mode_t mask;
	int    pflag = 0;
	int    mflag = 0;
	int    ret = 0;

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
	} ARGEND;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++) {
		if (pflag) {
			if (mkdirp(argv[0]) < 0)
				ret = 1;
		} else if (mkdir(argv[0], S_IRWXU|S_IRWXG|S_IRWXO) < 0) {
			weprintf("mkdir %s:", argv[0]);
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
