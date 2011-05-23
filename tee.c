/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	bool aflag = false;
	char buf[BUFSIZ];
	int i, nfps = 1;
	size_t n;
	FILE **fps;

	if(argc > 1 && !strcmp(argv[1], "-a"))
		aflag = true;
	if(!(fps = malloc(sizeof *fps)))
		eprintf("malloc:");
	fps[nfps-1] = stdout;

	for(i = aflag ? 2 : 1; i < argc; i++) {
		if(!(fps = realloc(fps, ++nfps * sizeof *fps)))
			eprintf("realloc:");
		if(!(fps[nfps-1] = fopen(argv[i], aflag ? "a" : "w")))
			eprintf("fopen %s:", argv[i]);
	}
	while((n = fread(buf, 1, sizeof buf, stdin)) > 0)
		for(i = 0; i < nfps; i++)
			if(fwrite(buf, 1, n, fps[i]) != n)
				eprintf("%s: write error:", buf);
	if(ferror(stdin))
		eprintf("<stdin>: read error:");
	return EXIT_SUCCESS;
}
