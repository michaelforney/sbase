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
			if(!(fp = fopen(argv[i], "r"))) {
				weprintf("fopen %s:", argv[i]);
				continue;
			}
			concat(fp, argv[i], stdout, "<stdout>");
			fclose(fp);
		}
	}
	return EXIT_SUCCESS;
}
