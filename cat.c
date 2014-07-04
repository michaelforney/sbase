/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *p;
	FILE *fp;
	int i;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc == 0) {
		concat(stdin, "<stdin>", stdout, "<stdout>");
	} else {
		for(i = 0; i < argc; i++) {
			p = argv[i];
			if (argv[i][0] == '-')
				p = "/dev/fd/0";
			if(!(fp = fopen(p, "r"))) {
				weprintf("fopen %s:", argv[i]);
				continue;
			}
			concat(fp, p, stdout, "<stdout>");
			fclose(fp);
		}
	}
	return EXIT_SUCCESS;
}
