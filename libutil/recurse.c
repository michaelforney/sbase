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
	char buf[PATH_MAX];
	struct dirent *d;
	struct stat lst, st;
	DIR *dp;

	if (lstat(path, &lst) < 0)
		eprintf("lstat %s:", path);
	if (stat(path, &st) < 0)
		eprintf("stat %s:", path);
	if (!S_ISDIR(lst.st_mode) && !(S_ISLNK(lst.st_mode) && S_ISDIR(st.st_mode) &&
	    !(recurse_follow == 'P' || (recurse_follow == 'H' && depth > 0))))
		return;

	if (!(dp = opendir(path)))
		eprintf("opendir %s:", path);

	while ((d = readdir(dp))) {
		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		if (strlcpy(buf, path, sizeof(buf)) >= sizeof(buf))
			eprintf("path too long\n");
		if (buf[strlen(buf) - 1] != '/')
			if (strlcat(buf, "/", sizeof(buf)) >= sizeof(buf))
				eprintf("path too long\n");
		if (strlcat(buf, d->d_name, sizeof(buf)) >= sizeof(buf))
			eprintf("path too long\n");
		fn(buf, depth + 1);
	}

	closedir(dp);
}
