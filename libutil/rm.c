/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../fs.h"
#include "../util.h"

int rm_status = 0;

void
rm(int dirfd, const char *name, struct stat *st, void *data, struct recursor *r)
{
	int quiet, ask, write, flags, ignore;

	ignore = r->flags & IGNORE;
	quiet = r->flags & SILENT;
	ask = r->flags & CONFIRM;
	write = faccessat(dirfd, name, W_OK, 0) == 0;
	flags = 0;

	if (S_ISDIR(st->st_mode) && r->maxdepth) {
		errno = EISDIR;
		goto err;
	}

	if (!quiet && (!write && isatty(0) || ask)) {
		if (!confirm("remove file '%s'? ", r->path))
			return;
	}

	if (S_ISDIR(st->st_mode)) {
		flags = AT_REMOVEDIR;
		if (r->depth)
			recurse(dirfd, name, NULL, r);
	}

	if (unlinkat(dirfd, name, flags) < 0)
		goto err;
	return;

err:
	if (!ignore) {
		weprintf("cannot remove '%s':", r->path);
		rm_status = 1;
	}
}
