/* See LICENSE file for copyright and license details. */
#include <unistd.h>

#include "../util.h"

char *
agetcwd(void)
{
	char *buf;
	long size;

	apathmax(&buf, &size);
	if (!getcwd(buf, size))
		eprintf("getcwd:");

	return buf;
}

