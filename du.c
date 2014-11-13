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
static long depth = -1;
static long curdepth = 0;

static bool aflag = false;
static bool dflag = false;
static bool sflag = false;
static bool kflag = false;
static bool hflag = false;

static long du(const char *);
static void print(long n, char *path);

static void
usage(void)
{
	eprintf("usage: %s [-a | -s] [-d depth] [-h] [-k] [file...]\n", argv0);
}

static char *
xrealpath(const char *pathname, char *resolved)
{
	char *r;

	r = realpath(pathname, resolved);
	if (!r)
		eprintf("realpath: %s:");
	return r;
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
	case 'd':
		dflag = true;
		depth = estrtol(EARGF(usage()), 0);
		break;
	case 's':
		sflag = true;
		break;
	case 'k':
		kflag = true;
		break;
	case 'h':
		hflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if ((aflag && sflag) || (dflag && sflag))
		usage();

	bsize = getenv("BLOCKSIZE");
	if (bsize)
		blksize = estrtol(bsize, 0);

	if (kflag)
		blksize = 1024;

	if (argc < 1) {
		n = du(".");
		if (sflag)
			print(n, xrealpath(".", file));
	} else {
		for (; argc > 0; argc--, argv++) {
			curdepth = 0;
			n = du(argv[0]);
			if (sflag)
				print(n, xrealpath(argv[0], file));
		}
	}
	return 0;
}

static void
print(long n, char *path)
{
	if (hflag)
		printf("%s\t%s\n", humansize(n * blksize), path);
	else
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
	long n = 0, m, t;
	int r;

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
			t = curdepth;
			curdepth++;
			n += du(dent->d_name);
			curdepth = t;
			continue;
		}
		m = nblks(&st);
		n += m;
		if (aflag && !sflag) {
			if (S_ISLNK(st.st_mode)) {
				r = snprintf(file, sizeof(file), "%s/%s",
					     cwd, dent->d_name);
				if (r >= sizeof(file) || r < 0)
					eprintf("path too long\n");
			} else {
				xrealpath(dent->d_name, file);
			}
			if (!dflag || (depth != -1 && curdepth < depth))
				print(m, file);
		}
	}
	pop(cwd);
	closedir(dp);

done:
	if (!sflag && (!dflag || (depth != -1 && curdepth <= depth)))
		print(n, xrealpath(path, file));
	return n;
}
