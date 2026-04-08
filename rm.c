/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include  <string.h>

#include "fs.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-f] [-iRr] file ...\n", argv0);
}

static int
forbidden(char *path, struct stat *root)
{
	char *s, *t;
	size_t n;
	struct stat st;
	static int w1, w2;

	n = strlen(path);
	for (t = path + n; t > path && t[-1] == '/'; --t)
		;
	for (s = t; s > path && s[-1] != '/'; --s)
		;
	n = t - s;
	if (n == 1 && *s == '.' || n == 2 && s[0] == '.' && s[1] == '.') {
		if (!w1)
			weprintf("\".\" and \"..\" may not be removed\n");
		w1 = 1;
		return 1;
	}

	if (stat(path, &st) < 0)
		return 0;
	if (st.st_dev == root->st_dev && st.st_ino == root->st_ino) {
		if (!w2)
			weprintf("\"/\" may not be removed\n");
		w2 = 1;
		return 1;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	char *s;
	struct stat st;
	struct recursor r = { .fn = rm, .maxdepth = 1, .follow = 'P' };

	ARGBEGIN {
	case 'f':
		r.flags |= SILENT | IGNORE;
		break;
	case 'i':
		r.flags |= CONFIRM;
		break;
	case 'R':
	case 'r':
		r.maxdepth = 0;
		break;
	default:
		usage();
	} ARGEND

	if (!argc) {
		if (!(r.flags & IGNORE))
			usage();
		else
			return 0;
	}

	if (stat("/", &st) < 0)
		eprintf("stat root:");
	for (; *argv; argc--, argv++) {
		if (forbidden(*argv, &st)) {
			rm_status = 1;
			continue;
		}
		recurse(AT_FDCWD, *argv, NULL, &r);
	}

	return rm_status || recurse_status;
}
