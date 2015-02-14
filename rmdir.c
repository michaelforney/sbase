/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: rmdir [-p] dir ...\n");
}

int
main(int argc, char *argv[])
{
	int   pflag = 0, ret = 0;
	char *d;

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++) {
		if (rmdir(argv[0]) < 0) {
			weprintf("rmdir %s:", argv[0]);
			ret = 1;
		}
		if (pflag && !ret) {
			d = dirname(argv[0]);
			for (; strcmp(d, "/") && strcmp(d, ".") ;) {
				if (rmdir(d) < 0)
					eprintf("rmdir %s:", d);
				d = dirname(d);
			}
		}
	}
	return ret;
}
