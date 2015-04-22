/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#include <sys/time.h>

#include <errno.h>
#include <grp.h>
#include <libgen.h>
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

#undef major
#define major(dev) ((int)(((unsigned int)(dev) >> 8) & 0xff))
#undef minor
#define minor(dev) ((int)((dev) & 0xff))
#undef makedev
#define makedev(major, minor) (((major) << 8) | (minor))

enum Type {
	REG = '0', AREG = '\0', HARDLINK = '1', SYMLINK = '2', CHARDEV = '3',
	BLOCKDEV = '4', DIRECTORY = '5', FIFO = '6'
};

static FILE *tarfile;
static char *tarfilename;
static ino_t tarinode;
static dev_t tardev;

static int  mflag;
static char filtermode = '\0';

static FILE *
decomp(FILE *fp)
{
	int fds[2];
	char *tool;

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

		tool = (filtermode == 'j') ? "bzip2" : "gzip";
		execlp(tool, tool, "-cd", NULL);
		weprintf("execlp %s:", tool);
		_exit(1);
	}
	close(fds[1]);

	return fdopen(fds[0], "r");
}

static void
putoctal(char *dst, unsigned num, int size)
{
	if (snprintf(dst, size, "%.*o", size - 1, num) >= size)
		eprintf("snprintf: input number too large\n");
}

static int
archive(const char *path)
{
	FILE *f = NULL;
	struct group *gr;
	struct header *h;
	struct passwd *pw;
	struct stat st;
	size_t chksum, x;
	ssize_t l, r;
	unsigned char b[BLKSIZ];

	if (lstat(path, &st) < 0) {
		weprintf("lstat %s:", path);
		return 0;
	} else if (st.st_ino == tarinode && st.st_dev == tardev) {
		weprintf("ignoring %s\n", path);
		return 0;
	}

	pw = getpwuid(st.st_uid);
	gr = getgrgid(st.st_gid);

	h = (struct header *)b;
	memset(b, 0, sizeof(b));
	estrlcpy(h->name,    path,                        sizeof(h->name));
	putoctal(h->mode,    (unsigned)st.st_mode & 0777, sizeof(h->mode));
	putoctal(h->uid,     (unsigned)st.st_uid,         sizeof(h->uid));
	putoctal(h->gid,     (unsigned)st.st_gid,         sizeof(h->gid));
	putoctal(h->size,    0,                           sizeof(h->size));
	putoctal(h->mtime,   (unsigned)st.st_mtime,       sizeof(h->mtime));
	memcpy(  h->magic,   "ustar",                     sizeof(h->magic));
	memcpy(  h->version, "00",                        sizeof(h->version));
	estrlcpy(h->uname,   pw ? pw->pw_name : "",       sizeof(h->uname));
	estrlcpy(h->gname,   gr ? gr->gr_name : "",       sizeof(h->gname));

	if (S_ISREG(st.st_mode)) {
		h->type = REG;
		putoctal(h->size, (unsigned)st.st_size,  sizeof(h->size));
		f = fopen(path, "r");
	} else if (S_ISDIR(st.st_mode)) {
		h->type = DIRECTORY;
	} else if (S_ISLNK(st.st_mode)) {
		h->type = SYMLINK;
		if ((r = readlink(path, h->link, sizeof(h->link) - 1)) < 0)
			eprintf("readlink %s:", path);
		h->link[r] = '\0';
	} else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
		h->type = S_ISCHR(st.st_mode) ? CHARDEV : BLOCKDEV;
		putoctal(h->major, (unsigned)major(st.st_dev), sizeof(h->major));
		putoctal(h->minor, (unsigned)minor(st.st_dev), sizeof(h->minor));
	} else if (S_ISFIFO(st.st_mode)) {
		h->type = FIFO;
	}

	memset(h->chksum, ' ', sizeof(h->chksum));
	for (x = 0, chksum = 0; x < sizeof(*h); x++)
		chksum += b[x];
	putoctal(h->chksum, chksum, sizeof(h->chksum));

	if (fwrite(b, BLKSIZ, 1, tarfile) != 1)
		eprintf("fwrite:");

	if (f) {
		while ((l = fread(b, 1, BLKSIZ, f)) > 0) {
			if (l < BLKSIZ)
				memset(b + l, 0, BLKSIZ - l);
			if (fwrite(b, BLKSIZ, 1, tarfile) != 1)
				eprintf("fwrite:");
		}
		efshut(f, path);
	}

	return 0;
}

static int
unarchive(char *fname, ssize_t l, char b[BLKSIZ])
{
	FILE *f = NULL;
	struct timeval times[2];
	struct header *h = (struct header *)b;
	long mode, major, minor, type, mtime, uid, gid;
	char lname[101], *tmp, *p;

	if (!mflag && ((mtime = strtol(h->mtime, &p, 8)) < 0 || *p != '\0'))
		eprintf("strtol %s: invalid number\n", h->mtime);
	if (unlink(fname) < 0 && errno != ENOENT && errno != EISDIR)
		eprintf("unlink %s:", fname);

	tmp = estrdup(fname);
	mkdirp(dirname(tmp));
	free(tmp);

	switch (h->type) {
	case REG:
	case AREG:
		if ((mode = strtol(h->mode, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->mode);
		if (!(f = fopen(fname, "w")))
			eprintf("fopen %s:", fname);
		if (chmod(fname, mode) < 0)
			eprintf("chmod %s:", fname);
		break;
	case HARDLINK:
	case SYMLINK:
		snprintf(lname, sizeof(lname), "%.*s", (int)sizeof(h->link),
		         h->link);
		if (((h->type == HARDLINK) ? link : symlink)(lname, fname) < 0)
			eprintf("%s %s -> %s:",
			        (h->type == HARDLINK) ? "link" : "symlink",
				fname, lname);
		break;
	case DIRECTORY:
		if ((mode = strtol(h->mode, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->mode);
		if (mkdir(fname, (mode_t)mode) < 0 && errno != EEXIST)
			eprintf("mkdir %s:", fname);
		break;
	case CHARDEV:
	case BLOCKDEV:
		if ((mode = strtol(h->mode, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->mode);
		if ((major = strtol(h->major, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->major);
		if ((minor = strtol(h->minor, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->minor);
		type = (h->type == CHARDEV) ? S_IFCHR : S_IFBLK;
		if (mknod(fname, type | mode, makedev(major, minor)) < 0)
			eprintf("mknod %s:", fname);
		break;
	case FIFO:
		if ((mode = strtol(h->mode, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->mode);
		if (mknod(fname, S_IFIFO | mode, 0) < 0)
			eprintf("mknod %s:", fname);
		break;
	default:
		eprintf("unsupported tar-filetype %c\n", h->type);
	}

	if ((uid = strtol(h->uid, &p, 8)) < 0 || *p != '\0')
		eprintf("strtol %s: invalid number\n", h->uid);
	if ((gid = strtol(h->gid, &p, 8)) < 0 || *p != '\0')
		eprintf("strtol %s: invalid number\n", h->gid);
	if (!getuid() && chown(fname, uid, gid))
		weprintf("chown %s:", fname);

	for (; l > 0; l -= BLKSIZ) {
		if (fread(b, BLKSIZ, 1, tarfile) != 1)
			eprintf("fread %s:", tarfilename);
		if (f && fwrite(b, MIN(l, BLKSIZ), 1, f) != 1)
			eprintf("fwrite %s:", fname);
	}
	if (f)
		fshut(f, fname);

	if (!mflag) {
		times[0].tv_sec = times[1].tv_sec = mtime;
		times[0].tv_usec = times[1].tv_usec = 0;
		if (utimes(fname, times) < 0)
			eprintf("utimes %s:", fname);
	}

	return 0;
}

static int
print(char *fname, ssize_t l, char b[BLKSIZ])
{
	puts(fname);

	for (; l > 0; l -= BLKSIZ)
		if (fread(b, BLKSIZ, 1, tarfile) != 1)
			eprintf("fread %s:", tarfilename);

	return 0;
}

static void
c(const char *path, struct stat *st, void *data, struct recursor *r)
{
	archive(path);

	if (st && S_ISDIR(st->st_mode))
		recurse(path, NULL, r);
}

static void
sanitize(struct header *h)
{
	size_t i, j;
	struct {
		char  *f;
		size_t l;
	} fields[] = {
		{ h->mode,   sizeof(h->mode)   },
		{ h->uid,    sizeof(h->uid)    },
		{ h->gid,    sizeof(h->gid)    },
		{ h->size,   sizeof(h->size)   },
		{ h->mtime,  sizeof(h->mtime)  },
		{ h->chksum, sizeof(h->chksum) },
		{ h->major,  sizeof(h->major)  },
		{ h->minor,  sizeof(h->minor)  }
	};

	/* Numeric fields can be terminated with spaces instead of
	 * NULs as per the ustar specification.  Patch all of them to
	 * use NULs so we can perform string operations on them. */
	for (i = 0; i < LEN(fields); i++)
		for (j = 0; j < fields[i].l; j++)
			if (fields[i].f[j] == ' ')
				fields[i].f[j] = '\0';
}

static void
xt(int argc, char *argv[], int (*fn)(char *, ssize_t, char[BLKSIZ]))
{
	char b[BLKSIZ], fname[256 + 1], *p;
	struct header *h = (struct header *)b;
	long size;
	int i, n;

	while (fread(b, BLKSIZ, 1, tarfile) == 1 && *h->name) {
		sanitize(h), n = 0;

		/* small dance around non-null terminated fields */
		if (h->prefix[0])
			n = snprintf(fname, sizeof(fname), "%.*s/",
			             (int)sizeof(h->prefix), h->prefix);
		snprintf(fname + n, sizeof(fname) - n, "%.*s",
		         (int)sizeof(h->name), h->name);

		if ((size = strtol(h->size, &p, 8)) < 0 || *p != '\0')
			eprintf("strtol %s: invalid number\n", h->size);

		if (argc) {
			/* only extract the given files */
			for (i = 0; i < argc; i++)
				if (!strcmp(argv[i], fname))
					break;
			if (i == argc) {
				for (; size > 0; size -= BLKSIZ)
					if (fread(b, BLKSIZ, 1, tarfile) != 1)
						eprintf("fread %s:", tarfilename);
				continue;
			}
		}

		fn(fname, size, b);
	}
	if (ferror(tarfile))
		eprintf("fread %s:", tarfilename);
}

static void
usage(void)
{
	eprintf("usage: %s [-C dir] [-j | -z] -x [-m | -t] [-f file] [file ...]\n"
		"       %s [-C dir] [-h] -c path ... [-f file]\n", argv0, argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	struct recursor r = { .fn = c, .hist = NULL, .depth = 0, .maxdepth = 0,
	                      .follow = 'P', .flags = DIRFIRST };
	struct stat st;
	char *file = NULL, *dir = ".", mode = '\0';

	ARGBEGIN {
	case 'x':
	case 'c':
	case 't':
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
		filtermode = ARGC();
		break;
	case 'h':
		r.follow = 'L';
		break;
	default:
		usage();
	} ARGEND;

	if (!mode)
		usage();
	if (mode == 'c')
		if (!argc || filtermode)
			usage();

	switch (mode) {
	case 'c':
		if (file) {
			if (!(fp = fopen(file, "w")))
				eprintf("fopen %s:", file);
			if (lstat(file, &st) < 0)
				eprintf("lstat %s:", file);
			tarinode = st.st_ino;
			tardev = st.st_dev;
			tarfile = fp;
			tarfilename = file;
		} else {
			tarfile = stdout;
			tarfilename = "<stdout>";
		}
		if (chdir(dir) < 0)
			eprintf("chdir %s:", dir);
		for (; *argv; argc--, argv++)
			recurse(*argv, NULL, &r);
		break;
	case 't':
	case 'x':
		if (file) {
			if (!(fp = fopen(file, "r")))
				eprintf("fopen %s:", file);
		} else {
			fp = stdin;
		}

		tarfilename = file;

		switch (filtermode) {
		case 'j':
		case 'z':
			tarfile = decomp(fp);
			break;
		default:
			tarfile = fp;
			break;
		}

		if (chdir(dir) < 0)
			eprintf("chdir %s:", dir);
		xt(argc, argv, (mode == 'x') ? unarchive : print);
		break;
	}

	return recurse_status;
}
