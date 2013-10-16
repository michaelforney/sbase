/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "util.h"

static long blksize = 512;

static bool aflag = false;

static long du(const char *);

void
usage(void)
{
	eprintf("usage: %s [-a] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *bsize;

	ARGBEGIN {
	case 'a':
		aflag = true;
		break;
	default:
		usage();
	} ARGEND;

	bsize = getenv("BLOCKSIZE");
	if (bsize)
		blksize = estrtol(bsize, 0);

	if (argc < 1) {
		du(".");
	} else {
		for (; argc > 0; argc--, argv++)
			du(argv[0]);
	}
	return EXIT_SUCCESS;
}

static void
print(long n, char *path)
{
	printf("%lu\t%s\n", n, path);
	free(path);
}

static char *
push(const char *path)
{
	char *cwd;

	cwd = agetcwd();
	if (chdir(path) < 0)
		eprintf("chdir: %s:", path);
	return cwd;
}

static void
pop(char *path)
{
	if (chdir(path) < 0)
		eprintf("chdir: %s:", path);
	free(path);
}

static long
du(const char *path)
{
	DIR *dp;
	char *cwd;
	struct dirent *dent;
	struct stat st;
	long n = 0, m;

	if (lstat(path, &st) < 0)
		eprintf("stat: %s:", path);
	n = 512 * st.st_blocks / blksize;

	if (S_ISDIR(st.st_mode)) {
		dp = opendir(path);
		if (!dp) {
			fprintf(stderr, "opendir: %s: %s\n", path,
				strerror(errno));
		} else {
			cwd = push(path);
			while ((dent = readdir(dp))) {
				if (strcmp(dent->d_name, ".") == 0 ||
				    strcmp(dent->d_name, "..") == 0)
					continue;
				if (lstat(dent->d_name, &st) < 0)
					eprintf("stat: %s:", dent->d_name);
				if (S_ISDIR(st.st_mode)) {
					n += du(dent->d_name);
				} else {
					m = 512 * st.st_blocks / blksize;
					n += m;
					if (aflag)
						print(m, realpath(dent->d_name, NULL));
				}
			}
			pop(cwd);
			closedir(dp);
		}
	}

	print(n, realpath(path, NULL));
	return n;
}
