/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-a] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	bool aflag = false;
	char buf[BUFSIZ];
	int i, nfps = 1;
	size_t n;
	FILE **fps = NULL;

	ARGBEGIN {
	case 'a':
		aflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if(!(fps = malloc(sizeof *fps)))
		eprintf("malloc:");
	fps[nfps-1] = stdout;

	for(; argc > 0; argc--, argv++) {
		if(!(fps = realloc(fps, ++nfps * sizeof *fps)))
			eprintf("realloc:");
		if(!(fps[nfps-1] = fopen(argv[0], aflag ? "a" : "w")))
			eprintf("fopen %s:", argv[0]);
	}
	while((n = fread(buf, 1, sizeof buf, stdin)) > 0) {
		for(i = 0; i < nfps; i++) {
			if(fwrite(buf, 1, n, fps[i]) != n)
				eprintf("%s: write error:", buf);
		}
	}
	if(ferror(stdin))
		eprintf("<stdin>: read error:");
	free(fps);

	return EXIT_SUCCESS;
}
