/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "util.h"

static long blksize = 512;
static char file[PATH_MAX];

static bool aflag = false;
static bool sflag = false;
static bool kflag = false;

static long du(const char *);
static void print(long n, char *path);

void
usage(void)
{
	eprintf("usage: %s [-a | -s] [-k] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *bsize;
	long n;

	ARGBEGIN {
	case 'a':
		aflag = true;
		break;
	case 's':
		sflag = true;
		break;
	case 'k':
		kflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if (aflag && sflag)
		usage();

	bsize = getenv("BLOCKSIZE");
	if (bsize)
		blksize = estrtol(bsize, 0);

	if (kflag)
		blksize = 1024;

	if (argc < 1) {
		n = du(".");
		if (sflag)
			print(n, realpath(".", file));
	} else {
		for (; argc > 0; argc--, argv++) {
			n = du(argv[0]);
			if (sflag)
				print(n, realpath(argv[0], file));
		}
	}
	return EXIT_SUCCESS;
}

static void
print(long n, char *path)
{
	printf("%lu\t%s\n", n, path);
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
nblks(struct stat *st)
{
	return (512 * st->st_blocks + blksize - 1) / blksize;
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
	n = nblks(&st);

	if (!S_ISDIR(st.st_mode))
		goto done;

	dp = opendir(path);
	if (!dp) {
		weprintf("opendir %s:", path);
		goto done;
	}

	cwd = push(path);
	while ((dent = readdir(dp))) {
		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0)
			continue;
		if (lstat(dent->d_name, &st) < 0)
			eprintf("stat: %s:", dent->d_name);
		if (S_ISDIR(st.st_mode)) {
			n += du(dent->d_name);
			continue;
		}
		m = nblks(&st);
		n += m;
		if (aflag && !sflag) {
			if (S_ISLNK(st.st_mode))
				snprintf(file, sizeof(file), "%s/%s",
					 cwd, dent->d_name);
			else
				realpath(dent->d_name, file);
			print(m, file);
		}
	}
	pop(cwd);
	closedir(dp);

done:
	if (!sflag)
		print(n, realpath(path, file));
	return n;
}
