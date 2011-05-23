/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	unsigned int seconds;

	if(argc != 2)
		eprintf("usage: %s seconds\n", argv[0]);

	seconds = atoi(argv[1]);
	while((seconds = sleep(seconds)) > 0)
		;
	return EXIT_SUCCESS;
}
