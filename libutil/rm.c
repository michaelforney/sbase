/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "../fs.h"
#include "../util.h"

int rm_status = 0;

void
rm(const char *path, struct stat *st, void *data, struct recursor *r)
{
	if (!r->maxdepth && S_ISDIR(st->st_mode)) {
		recurse(path, NULL, r);

		if (rmdir(path) < 0) {
			if (!(r->flags & SILENT))
				weprintf("rmdir %s:", path);
			if (!((r->flags & SILENT) && errno == ENOENT))
				rm_status = 1;
		}
	} else if (unlink(path) < 0) {
		if (!(r->flags & SILENT))
			weprintf("unlink %s:", path);
		if (!((r->flags & SILENT) && errno == ENOENT))
			rm_status = 1;
	}
}
