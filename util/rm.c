/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include "../fs.h"
#include "../util.h"

bool rm_fflag = false;
bool rm_rflag = false;

void
rm(const char *path)
{
	if(rm_rflag)
		recurse(path, rm);
	if(remove(path) == -1 && !rm_fflag)
		eprintf("remove %s:", path);
}
