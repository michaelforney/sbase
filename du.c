/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <search.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

static size_t maxdepth = SIZE_MAX;
static size_t blksize  = 512;

static int aflag = 0;
static int sflag = 0;
static int hflag = 0;

struct file {
	dev_t devno;
	ino_t inode;
};

static void
printpath(off_t n, const char *path)
{
	if (hflag)
		printf("%s\t%s\n", humansize(n * blksize), path);
	else
		printf("%jd\t%s\n", (intmax_t)n, path);
}

static off_t
nblks(blkcnt_t blocks)
{
	return (512 * blocks + blksize - 1) / blksize;
}

static int
cmp(const void *p1, const void *p2)
{
	const struct file *f1 = p1, *f2 = p2;

	if (f1->devno > f2->devno)
		return -1;
	if (f1->devno < f2->devno)
		return 1;

	/* f1->devno == f2->devno */
	if (f1->inode < f2->inode)
		return -1;
	if (f1->inode > f2->inode)
		return 1;

	return 0;
}

static int
duplicated(dev_t dev, ino_t ino)
{
	static void *tree;
	struct file **fpp, *fp, file = {dev, ino};

	if ((fpp = tsearch(&file, &tree, cmp)) == NULL)
		eprintf("%s:", argv0);

	if (*fpp != &file)
		return 1;

	/* new file added */
	fp = emalloc(sizeof(*fp));
	*fp = file;
	*fpp = fp;

	return 0;
}

static void
du(int dirfd, const char *path, struct stat *st, void *data, struct recursor *r)
{
	off_t *total = data, subtotal;

	subtotal = nblks(st->st_blocks);
	if (S_ISDIR(st->st_mode)) {
		recurse(dirfd, path, &subtotal, r);
	} else if (r->follow != 'P' || st->st_nlink > 1) {
		if (duplicated(st->st_dev, st->st_ino))
			goto print;
	}

	*total += subtotal;

print:
	if (!r->depth)
		printpath(*total, r->path);
	else if (!sflag && r->depth <= maxdepth && (S_ISDIR(st->st_mode) || aflag))
		printpath(subtotal, r->path);
}

static void
usage(void)
{
	eprintf("usage: %s [-a | -s] [-d depth] [-h] [-k] [-H | -L | -P] [-x] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct recursor r = { .fn = du, .follow = 'P' };
	off_t n = 0;
	int kflag = 0, dflag = 0;
	char *bsize;

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 'd':
		dflag = 1;
		maxdepth = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
		break;
	case 'h':
		hflag = 1;
		break;
	case 'k':
		kflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'x':
		r.flags |= SAMEDEV;
		break;
	case 'H':
	case 'L':
	case 'P':
		r.follow = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if ((aflag && sflag) || (dflag && sflag))
		usage();

	bsize = getenv("BLOCKSIZE");
	if (bsize)
		blksize = estrtonum(bsize, 1, MIN(LLONG_MAX, SIZE_MAX));
	if (kflag)
		blksize = 1024;

	if (!argc) {
		recurse(AT_FDCWD, ".", &n, &r);
	} else {
		for (; *argv; argc--, argv++) {
			n = 0;
			recurse(AT_FDCWD, *argv, &n, &r);
		}
	}

	return fshut(stdout, "<stdout>") || recurse_status;
}
