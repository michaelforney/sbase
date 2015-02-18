/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static size_t blksize = 512;
static char   file[PATH_MAX];
static size_t depth = -1;
static size_t curdepth = 0;

static int aflag = 0;
static int dflag = 0;
static int sflag = 0;
static int kflag = 0;
static int hflag = 0;
static int xflag = 0;
static int HLPflag = 'P';

static char *
xrealpath(const char *pathname, char *resolved)
{
	char *r;

	r = realpath(pathname, resolved);
	if (!r)
		eprintf("realpath: %s:", pathname);
	return r;
}

static void
print(size_t n, char *path)
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
		weprintf("chdir: %s:", path);
	return cwd;
}

static void
pop(char *path)
{
	if (chdir(path) < 0)
		weprintf("chdir: %s:", path);
	free(path);
}

static size_t
nblks(struct stat *st)
{
	return (512 * st->st_blocks + blksize - 1) / blksize;
}

static size_t
du(const char *path, char follow)
{
	struct dirent *dent;
	struct stat pst, st;
	DIR *dp;
	size_t n = 0, m, t;
	int r;
	char *cwd;

	if (lstat(path, &pst) < 0)
		eprintf("stat: %s:", path);
	n = nblks(&pst);

	if (!(S_ISDIR(pst.st_mode) ||
	    (follow != 'P' && S_ISLNK(pst.st_mode) &&
	    stat(path, &pst) == 0 && S_ISDIR(pst.st_mode))))
		goto done;

	follow = follow == 'H' ? 'P' : follow;

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
		if (xflag && st.st_dev != pst.st_dev)
			continue;
		if (S_ISDIR(st.st_mode) ||
		    (follow != 'P' && S_ISLNK(st.st_mode) &&
		     stat(dent->d_name, &st) == 0 && S_ISDIR(st.st_mode))) {
			t = curdepth;
			curdepth++;
			n += du(dent->d_name, follow);
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

static void
usage(void)
{
	eprintf("usage: %s [-a | -s] [-d depth] [-h] [-k] [-H | -L | -P] [-x] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	size_t n;
	char *bsize;

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 'd':
		dflag = 1;
		depth = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
		break;
	case 's':
		sflag = 1;
		break;
	case 'k':
		kflag = 1;
		break;
	case 'h':
		hflag = 1;
		break;
	case 'H':
	case 'L':
	case 'P':
		HLPflag = ARGC();
		break;
	case 'x':
		xflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if ((aflag && sflag) || (dflag && sflag))
		usage();

	bsize = getenv("BLOCKSIZE");
	if (bsize)
		blksize = estrtonum(bsize, 0, LONG_MAX);

	if (kflag)
		blksize = 1024;

	if (argc < 1) {
		n = du(".", HLPflag);
		if (sflag)
			print(n, xrealpath(".", file));
	} else {
		for (; argc > 0; argc--, argv++) {
			curdepth = 0;
			n = du(argv[0], HLPflag);
			if (sflag)
				print(n, xrealpath(argv[0], file));
		}
	}
	return 0;
}
