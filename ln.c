/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static int ln(const char *, const char *);

static bool sflag = false;
static bool fflag = false;

int
main(int argc, char *argv[])
{
	char c;

	while((c = getopt(argc, argv, "fs")) != -1)
		switch(c) {
		case 'f':
			fflag = true;
			break;
		case 's':
			sflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	enmasse(argc - optind, &argv[optind], ln);
	return EXIT_SUCCESS;
}

int
ln(const char *s1, const char *s2)
{
	if(fflag && remove(s2) != 0 && errno != ENOENT)
		eprintf("remove %s:", s2);
	return (sflag ? symlink : link)(s1, s2);
}
