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

	if(getopt(argc, argv, "") != -1)
		exit(EXIT_FAILURE);
	if(optind == argc)
		concat(stdin, "<stdin>", stdout, "<stdout>");
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		concat(fp, argv[optind], stdout, "<stdout>");
		fclose(fp);
	}
	return EXIT_SUCCESS;
}
