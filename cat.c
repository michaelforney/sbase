/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-u] [file...]\n", argv0);
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

	if (argc == 0) {
		concat(stdin, "<stdin>", stdout, "<stdout>");
	} else {
		for (; argc; argc--, argv++) {
			if (argv[0][0] == '-' && !argv[0][1])
				argv[0] = "/dev/fd/0";
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				ret = 1;
				continue;
			}
			concat(fp, argv[0], stdout, "<stdout>");
			fclose(fp);
		}
	}
	return ret;
}
