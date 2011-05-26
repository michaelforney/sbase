/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../util.h"

void
recurse(const char *path, void (*fn)(const char *))
{
	char *cwd;
	struct dirent *d;
	DIR *dp;

	if(!(dp = opendir(path))) {
		if(errno == ENOTDIR)
			return;
		else
			eprintf("opendir %s:", path);
	}
	cwd = agetcwd();
	if(chdir(path) != 0)
		eprintf("chdir %s:", path);
	while((d = readdir(dp)))
		if(strcmp(d->d_name, ".") && strcmp(d->d_name, ".."))
			fn(d->d_name);

	closedir(dp);
	if(chdir(cwd) != 0)
		eprintf("chdir %s:", cwd);
	free(cwd);
}
