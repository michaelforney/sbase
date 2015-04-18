/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "../fs.h"
#include "../util.h"

int rm_fflag = 0;
int rm_status = 0;

void
rm(const char *path, struct stat *st, void *data, struct recursor *r)
{
	if (!r->maxdepth && st && S_ISDIR(st->st_mode)) {
		recurse(path, NULL, r);

		if (rmdir(path) < 0) {
			if (!rm_fflag)
				weprintf("rmdir %s:", path);
			if (!(rm_fflag && errno == ENOENT))
				rm_status = 1;
		}
	} else if (unlink(path) < 0) {
		if (!rm_fflag)
			weprintf("unlink %s:", path);
		if (!(rm_fflag && errno == ENOENT))
			rm_status = 1;
	}
}
