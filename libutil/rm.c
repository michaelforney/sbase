/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "../fs.h"
#include "../util.h"

int rm_fflag = 0;
int rm_rflag = 0;
int rm_status = 0;

void
rm(const char *path, int unused)
{
	if (rm_rflag)
		recurse(path, rm, 'P');
	if (remove(path) < 0) {
		if (!rm_fflag)
			weprintf("remove %s:", path);
		rm_status = 1;
	}
}
