/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fs.h"
#include "util.h"

int
main(int argc, char *argv[])
{
	char c;
	struct stat st;

	while((c = getopt(argc, argv, "fr")) != -1)
		switch(c) {
		case 'f':
			rm_fflag = true;
			break;
		case 'r':
			rm_rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	for(; optind < argc; optind++) {
		if(!rm_rflag && stat(argv[optind], &st) == 0 &&
				S_ISDIR(st.st_mode))
			fprintf(stderr, "%s: is a directory\n", argv[optind]);
		else
			rm(argv[optind]);
	}
	return EXIT_SUCCESS;
}
