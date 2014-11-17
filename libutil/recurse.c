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

void
recurse(const char *path, void (*fn)(const char *))
{
	char buf[PATH_MAX];
	struct dirent *d;
	struct stat st;
	DIR *dp;

	if (lstat(path, &st) == -1 || S_ISDIR(st.st_mode) == 0)
		return;

	if (!(dp = opendir(path)))
		eprintf("opendir %s:", path);

	while ((d = readdir(dp))) {
		if (strcmp(d->d_name, ".") == 0 ||
		    strcmp(d->d_name, "..") == 0)
			continue;
		if (strlcpy(buf, path, sizeof(buf)) >= sizeof(buf))
			eprintf("path too long\n");
		if (buf[strlen(buf) - 1] != '/')
			if (strlcat(buf, "/", sizeof(buf)) >= sizeof(buf))
				eprintf("path too long\n");
		if (strlcat(buf, d->d_name, sizeof(buf)) >= sizeof(buf))
			eprintf("path too long\n");
		fn(buf);
	}

	closedir(dp);
}
