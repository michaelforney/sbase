/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	bool sflag = false;
	char c;

	while((c = getopt(argc, argv, "s")) != -1)
		switch(c) {
		case 's':
			sflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	enmasse(argc - optind, &argv[optind], sflag ? symlink : link);
	return EXIT_SUCCESS;
}
