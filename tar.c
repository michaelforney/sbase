/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/time.h>

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

struct header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char type;
	char link[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char major[8];
	char minor[8];
	char prefix[155];
};

#define BLKSIZ 512

enum Type {
	REG = '0', AREG = '\0', HARDLINK = '1', SYMLINK = '2', CHARDEV = '3',
	BLOCKDEV = '4', DIRECTORY = '5', FIFO = '6'
};

static FILE *tarfile;
static ino_t tarinode;
static dev_t tardev;

static int  mflag;
static char filtermode;

static FILE *
decomp(FILE *fp)
{
	int   fds[2];

	if (pipe(fds) < 0)
		eprintf("pipe:");

	switch (fork()) {
	case -1:
		eprintf("fork:");
	case 0:
		dup2(fileno(fp), 0);
		dup2(fds[1], 1);
		close(fds[0]);
		close(fds[1]);

		switch (filtermode) {
		case 'j':
			execlp("bzip2", "bzip2", "-cd", NULL);
			weprintf("execlp bzip2:");
			_exit(1);
		case 'z':
			execlp("gzip", "gzip", "-cd", NULL);
			weprintf("execlp gzip:");
			_exit(1);
		}
	}
	close(fds[1]);
	return fdopen(fds[0], "r");
}

static void
putoctal(char *dst, unsigned num, int n)
{
	snprintf(dst, n, "%.*o", n - 1, num);
}

static int
archive(const char* path)
{
	FILE *f = NULL;
	mode_t mode;
	struct group *gr;
	struct header *h;
	struct passwd *pw;
	struct stat st;
	size_t chksum, x;
	ssize_t l;
	unsigned char b[BLKSIZ];

	lstat(path, &st);
	if (st.st_ino == tarinode && st.st_dev == tardev) {
		fprintf(stderr, "ignoring '%s'\n", path);
		return 0;
	}
	pw = getpwuid(st.st_uid);
	gr = getgrgid(st.st_gid);

	h = (void*)b;
	memset(b, 0, sizeof(b));
	snprintf(h->name, sizeof(h->name), "%s", path);
	putoctal(h->mode,  (unsigned)st.st_mode & 0777, sizeof(h->mode));
	putoctal(h->uid,   (unsigned)st.st_uid,         sizeof(h->uid));
	putoctal(h->gid,   (unsigned)st.st_gid,         sizeof(h->gid));
	putoctal(h->size,  0,                           sizeof(h->size));
	putoctal(h->mtime, (unsigned)st.st_mtime,       sizeof(h->mtime));
	memcpy(h->magic,   "ustar",                     sizeof(h->magic));
	memcpy(h->version, "00",                        sizeof(h->version));
	snprintf(h->uname, sizeof h->uname, "%s", pw ? pw->pw_name : "");
	snprintf(h->gname, sizeof h->gname, "%s", gr ? gr->gr_name : "");

	mode = st.st_mode;
	if (S_ISREG(mode)) {
		h->type = REG;
		putoctal(h->size, (unsigned)st.st_size,  sizeof h->size);
		f = fopen(path, "r");
	} else if (S_ISDIR(mode)) {
		h->type = DIRECTORY;
	} else if (S_ISLNK(mode)) {
		h->type = SYMLINK;
		readlink(path, h->link, (sizeof h->link)-1);
	} else if (S_ISCHR(mode) || S_ISBLK(mode)) {
		h->type = S_ISCHR(mode) ? CHARDEV : BLOCKDEV;
#if defined(major) && defined(minor)
		putoctal(h->major, (unsigned)major(st.st_dev), sizeof h->major);
		putoctal(h->minor, (unsigned)minor(st.st_dev), sizeof h->minor);
#else
		return 0;
#endif
	} else if (S_ISFIFO(mode)) {
		h->type = FIFO;
	}

	memset(h->chksum, ' ', sizeof h->chksum);
	for (x = 0, chksum = 0; x < sizeof *h; x++)
		chksum += b[x];
	putoctal(h->chksum, chksum, sizeof h->chksum);

	fwrite(b, BLKSIZ, 1, tarfile);
	if (!f)
		return 0;
	while ((l = fread(b, 1, BLKSIZ, f)) > 0) {
		if (l < BLKSIZ)
			memset(b+l, 0, BLKSIZ-l);
		fwrite(b, BLKSIZ, 1, tarfile);
	}
	fclose(f);
	return 0;
}

static int
unarchive(char *fname, int l, char b[BLKSIZ])
{
	FILE *f = NULL;
	struct timeval times[2];
	struct header *h = (void*)b;
	unsigned long mode, major, minor, type, mtime;
	char lname[101];

	if (!mflag)
		mtime = strtoul(h->mtime, 0, 8);
	unlink(fname);
	switch (h->type) {
	case REG:
	case AREG:
		mode = strtoul(h->mode, 0, 8);
		if (!(f = fopen(fname, "w")) || chmod(fname, mode))
			perror(fname);
		break;
	case HARDLINK:
	case SYMLINK:
		snprintf(lname, sizeof lname, "%s", h->link);
		if (!((h->type == HARDLINK) ? link : symlink)(lname, fname))
			perror(fname);
		break;
	case DIRECTORY:
		mode = strtoul(h->mode, 0, 8);
		if (mkdir(fname, (mode_t)mode))
			perror(fname);
		break;
	case CHARDEV:
	case BLOCKDEV:
#ifdef makedev
		mode = strtoul(h->mode, 0, 8);
		major = strtoul(h->major, 0, 8);
		minor = strtoul(h->mode, 0, 8);
		type = (h->type == CHARDEV) ? S_IFCHR : S_IFBLK;
		if (mknod(fname, type | mode, makedev(major, minor)))
			perror(fname);
#endif
		break;
	case FIFO:
		mode = strtoul(h->mode, 0, 8);
		if (mknod(fname, S_IFIFO | mode, 0))
			perror(fname);
		break;
	default:
		fprintf(stderr, "usupported tarfiletype %c\n", h->type);
	}
	if (getuid() == 0 && chown(fname, strtoul(h->uid, 0, 8), strtoul(h->gid, 0, 8)))
		perror(fname);

	for (; l > 0; l -= BLKSIZ) {
		fread(b, BLKSIZ, 1, tarfile);
		if (f)
			fwrite(b, MIN(l, 512), 1, f);
	}
	if (f)
		fclose(f);

	if (!mflag) {
		times[0].tv_sec = times[1].tv_sec = mtime;
		times[0].tv_usec = times[1].tv_usec = 0;
		if (utimes(fname, times))
			perror(fname);
	}
	return 0;
}

static int
print(char * fname, int l, char b[BLKSIZ])
{
	puts(fname);
	for (; l > 0; l -= BLKSIZ)
		fread(b, BLKSIZ, 1, tarfile);
	return 0;
}

static void
c(const char *path, void *data, struct recursor *r)
{
	archive(path);
	recurse(path, NULL, r);
}

static void
xt(int (*fn)(char *, int, char[BLKSIZ]))
{
	char b[BLKSIZ], fname[257], *s;
	struct header *h = (void*)b;

	while (fread(b, BLKSIZ, 1, tarfile) && h->name[0] != '\0') {
		s = fname;
		if (h->prefix[0] != '\0')
			s += sprintf(s, "%.*s/", (int)sizeof h->prefix, h->prefix);
		sprintf(s, "%.*s", (int)sizeof h->name, h->name);
		fn(fname, strtol(h->size, 0, 8), b);
	}
}

static void
usage(void)
{
	eprintf("usage: tar [-f tarfile] [-C dir] -j|z -x[m]|t\n"
	        "       tar [-f tarfile] [-C dir] -c dir\n");
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	struct recursor r = { .fn = c, .hist = NULL, .depth = 0, .follow = 'P', .flags = 0};
	struct stat st;
	char *file = NULL, *dir = ".", mode = '\0';

	ARGBEGIN {
	case 'x':
	case 'c':
	case 't':
		if (mode)
			usage();
		mode = ARGC();
		break;
	case 'C':
		dir = EARGF(usage());
		break;
	case 'f':
		file = EARGF(usage());
		break;
	case 'm':
		mflag = 1;
		break;
	case 'j':
	case 'z':
		if (filtermode)
			usage();
		filtermode = ARGC();
		break;
	case 'h':
		r.follow = 'L';
		break;
	default:
		usage();
	} ARGEND;

	if (!mode || argc != (mode == 'c'))
		usage();

	switch (mode) {
	case 'c':
		if (file) {
			if (!(fp = fopen(file, "w")))
				eprintf("fopen %s:", file);
			if (lstat(file, &st) < 0)
				eprintf("tar: stat '%s':", file);
			tarinode = st.st_ino;
			tardev = st.st_dev;
			tarfile = fp;
		} else {
			tarfile = stdout;
		}
		chdir(dir);
		c(argv[0], NULL, &r);
		break;
	case 't':
	case 'x':
		if (file) {
			if (!(fp = fopen(file, "r")))
				eprintf("fopen %s:", file);
		} else {
			fp = stdin;
		}

		switch (filtermode) {
		case 'j':
		case 'z':
			tarfile = decomp(fp);
			break;
		default:
			tarfile = fp;
			break;
		}

		chdir(dir);
		xt(mode == 'x' ? unarchive : print);
		break;
	}

	return recurse_status;
}
