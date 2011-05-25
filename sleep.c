/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	char *end;
	unsigned int seconds;

	if(getopt(argc, argv, "") != -1)
		exit(EXIT_FAILURE);
	if(optind != argc-1)
		eprintf("usage: %s seconds\n", argv[0]);

	seconds = strtol(argv[optind], &end, 0);
	if(*end != '\0')
		eprintf("%s: not a number\n", argv[optind]);
	while((seconds = sleep(seconds)) > 0)
		;
	return EXIT_SUCCESS;
}
