/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../util.h"

int recurse_follow  = 'P';
int recurse_samedev = 0;

void
recurse(const char *path, void (*fn)(const char *, int, void *), int depth, void *data)
{
	struct dirent *d;
	struct stat lst, st, dst;
	DIR *dp;
	size_t len;
	char *buf;

	if (lstat(path, &lst) < 0) {
		if (errno != ENOENT)
			weprintf("lstat %s:", path);
		return;
	}
	if (stat(path, &st) < 0) {
		if (errno != ENOENT)
			weprintf("stat %s:", path);
		return;
	}
	if (!S_ISDIR(lst.st_mode) && !(S_ISLNK(lst.st_mode) && S_ISDIR(st.st_mode) &&
	    !(recurse_follow == 'P' || (recurse_follow == 'H' && depth > 0))))
		return;

	if (!(dp = opendir(path)))
		eprintf("opendir %s:", path);

	len = strlen(path);
	while ((d = readdir(dp))) {
		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		buf = emalloc(len + (path[len - 1] != '/') + strlen(d->d_name) + 1);
		sprintf(buf, "%s%s%s", path, (path[len - 1] == '/') ? "" : "/", d->d_name);
		if (recurse_samedev && lstat(buf, &dst) < 0) {
			if (errno != ENOENT)
				weprintf("stat %s:", buf);
		} else if (!(recurse_samedev && dst.st_dev != lst.st_dev))
			fn(buf, depth + 1, data);
		free(buf);
	}

	closedir(dp);
}
