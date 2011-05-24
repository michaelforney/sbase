/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	bool aflag = false;
	char buf[BUFSIZ], c;
	int i, nfps = 1;
	size_t n;
	FILE **fps;

	while((c = getopt(argc, argv, "a")) != -1)
		switch(c) {
		case 'a':
			aflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(!(fps = malloc(sizeof *fps)))
		eprintf("malloc:");
	fps[nfps-1] = stdout;

	for(; optind < argc; optind++) {
		if(!(fps = realloc(fps, ++nfps * sizeof *fps)))
			eprintf("realloc:");
		if(!(fps[nfps-1] = fopen(argv[optind], aflag ? "a" : "w")))
			eprintf("fopen %s:", argv[optind]);
	}
	while((n = fread(buf, 1, sizeof buf, stdin)) > 0)
		for(i = 0; i < nfps; i++)
			if(fwrite(buf, 1, n, fps[i]) != n)
				eprintf("%s: write error:", buf);
	if(ferror(stdin))
		eprintf("<stdin>: read error:");
	return EXIT_SUCCESS;
}
