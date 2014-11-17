/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../util.h"

void
apathmax(char **p, long *size)
{
	errno = 0;

	if ((*size = pathconf("/", _PC_PATH_MAX)) == -1) {
		if (errno == 0) {
			*size = BUFSIZ;
		} else {
			eprintf("pathconf:");
		}
	}
	*p = emalloc(*size);
}
