/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../util.h"

void
recurse(const char *path, void (*fn)(const char *))
{
	char *cwd;
	struct dirent *d;
	struct stat st;
	DIR *dp;

	if(lstat(path, &st) == -1 || !S_ISDIR(st.st_mode))
		return;
	else if(!(dp = opendir(path)))
		eprintf("opendir %s:", path);

	cwd = agetcwd();
	if(chdir(path) == -1)
		eprintf("chdir %s:", path);
	while((d = readdir(dp)))
		if(strcmp(d->d_name, ".") && strcmp(d->d_name, ".."))
			fn(d->d_name);

	closedir(dp);
	if(chdir(cwd) == -1)
		eprintf("chdir %s:", cwd);
	free(cwd);
}
