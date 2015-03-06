/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include "../util.h"

void
apathmax(char **p, size_t *size)
{
	errno = 0;
	if ((*size = pathconf("/", _PC_PATH_MAX)) < 0) {
		if (errno == 0) {
			*size = PATH_MAX;
		} else {
			eprintf("pathconf:");
		}
	} else {
		(*size)++;
	}
	*p = emalloc(*size);
}
