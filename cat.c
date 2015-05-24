/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
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
			if (!strcmp(*argv, "-")) {
				*argv = "<stdin>";
				fp = stdin;
			} else if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			concat(fp, *argv, stdout, "<stdout>");
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
		}
	}

	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
