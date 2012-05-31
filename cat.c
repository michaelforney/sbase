/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

int
main(int argc, char *argv[])
{
	FILE *fp;
	int i;

	ARGBEGIN {
	default:
		eprintf("usage: %s [files...]\n", argv0);
	} ARGEND;

	if(argc == 0)
		concat(stdin, "<stdin>", stdout, "<stdout>");
	else for(i = 0; i < argc; i++) {
		if(!(fp = fopen(argv[i], "r")))
			eprintf("fopen %s:", argv[i]);
		concat(fp, argv[i], stdout, "<stdout>");
		fclose(fp);
	}
	return EXIT_SUCCESS;
}
