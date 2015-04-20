/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <limits.h>

#include "../util.h"

int
mkdirp(const char *path)
{
	char tmp[PATH_MAX], *p;

	estrlcpy(tmp, path, sizeof(tmp));
	for (p = tmp + (tmp[0] == '/'); *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		if (mkdir(tmp, S_IRWXU | S_IRWXG | S_IRWXO) < 0 && errno != EEXIST) {
			weprintf("mkdir %s:", tmp);
			return -1;
		}
		*p = '/';
	}
	if (mkdir(tmp, S_IRWXU | S_IRWXG | S_IRWXO) < 0 && errno != EEXIST) {
		weprintf("mkdir %s:", tmp);
		return -1;
	}
	return 0;
}
