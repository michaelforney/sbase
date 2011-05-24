/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void rm(const char *);

static bool fflag = false;
static bool rflag = false;

int
main(int argc, char *argv[])
{
	char c;

	while((c = getopt(argc, argv, "fr")) != -1)
		switch(c) {
		case 'f':
			fflag = true;
			break;
		case 'r':
			rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	for(; optind < argc; optind++)
		rm(argv[optind]);
	return EXIT_SUCCESS;
}

void
rm(const char *path)
{
	if(rflag)
		recurse(path, rm);
	if(remove(path) != 0 && !fflag)
		eprintf("remove %s:", path);
}
