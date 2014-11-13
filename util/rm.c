/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "../fs.h"
#include "../util.h"

int rm_fflag = 0;
int rm_rflag = 0;

void
rm(const char *path)
{
	if (rm_rflag)
		recurse(path, rm);
	if (remove(path) == -1 && !rm_fflag)
		eprintf("remove %s:", path);
}
