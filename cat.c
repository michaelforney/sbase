/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
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
	char *p;
	FILE *fp;
	int ret = 0;

	ARGBEGIN {
	case 'u':
		setbuf(stdout, NULL);
		break;
	default:
		usage();
	} ARGEND;

	if(argc == 0) {
		concat(stdin, "<stdin>", stdout, "<stdout>");
	} else {
		for (; argc; argc--, argv++) {
			p = argv[0];
			if (argv[0][0] == '-')
				p = "/dev/fd/0";
			if(!(fp = fopen(p, "r"))) {
				weprintf("fopen %s:", p);
				ret = 1;
				continue;
			}
			concat(fp, p, stdout, "<stdout>");
			fclose(fp);
		}
	}
	return ret;
}
