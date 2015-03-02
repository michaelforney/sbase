/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../util.h"

int recurse_follow = 'P';

void
recurse(const char *path, void (*fn)(const char *, int), int depth)
{
	struct dirent *d;
	struct stat lst, st;
	DIR *dp;
	size_t len;
	char *buf;

	if (lstat(path, &lst) < 0)
		eprintf("lstat %s:", path);
	if (stat(path, &st) < 0)
		eprintf("stat %s:", path);
	if (!S_ISDIR(lst.st_mode) && !(S_ISLNK(lst.st_mode) && S_ISDIR(st.st_mode) &&
	    !(recurse_follow == 'P' || (recurse_follow == 'H' && depth > 0))))
		return;

	if (!(dp = opendir(path)))
		eprintf("opendir %s:", path);

	len = strlen(path);
	while ((d = readdir(dp))) {
		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		buf = emalloc(len + (path[len] != '/') + strlen(d->d_name) + 1);
		sprintf(buf, "%s%s%s", path, (path[len] == '/') ? "" : "/", d->d_name);
		fn(buf, depth + 1);
		free(buf);
	}

	closedir(dp);
}
