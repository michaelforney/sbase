/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-u] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;

	ARGBEGIN {
	case 'u':
		setbuf(stdout, NULL);
		break;
	default:
		usage();
	} ARGEND;

	if (!argc) {
		concat(stdin, "<stdin>", stdout, "<stdout>");
	} else {
		for (; *argv; argc--, argv++) {
			if ((*argv)[0] == '-' && !(*argv)[1]) {
				concat(stdin, "<stdin>", stdout, "<stdout>");
			} else if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
			} else {
				concat(fp, *argv, stdout, "<stdout>");
				if (fshut(fp, *argv)) {
					ret = 1;
				}
			}
		}
	}

	return !!(fshut(stdin, "<stdin>") + fshut(stdout, "<stdout>")) || ret;
}
