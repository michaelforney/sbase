/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int
main(void)
{
	char *buf;
	long size;

	if((size = pathconf(".", _PC_PATH_MAX)) < 0)
		size = BUFSIZ;
	if(!(buf = malloc(size)))
		eprintf("malloc:");
	if(!getcwd(buf, size))
		eprintf("getcwd:");

	puts(buf);
	return EXIT_SUCCESS;
}
