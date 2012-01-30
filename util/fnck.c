/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include "../util.h"

void
fnck(const char *a, const char *b, int (*fn)(const char *, const char *))
{
	struct stat sta, stb;

	if(stat(a, &sta) == 0 && stat(b, &stb) == 0
	&& sta.st_dev == stb.st_dev && sta.st_ino == stb.st_ino)
		eprintf("%s -> %s: same file\n", a, b);
	if(fn(a, b) == -1)
		eprintf("%s -> %s:", a, b);
}
