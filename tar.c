/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <ftw.h>
#include <grp.h>
#include <pwd.h>
#include "util.h"

typedef struct Header Header;
struct Header {
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
};

enum {
	Blksiz = 512
};

enum Type {
	REG = '0', HARDLINK = '1', SYMLINK = '2', CHARDEV = '3',
	BLOCKDEV = '4', DIRECTORY = '5', FIFO = '6'
};

static void putoctal(char *, unsigned, int);
static int archive(const char *, const struct stat *, int);
static int unarchive(char *, int, char[Blksiz]);
static int print(char *, int , char[Blksiz]);
static void c(char *);
static void xt(int (*)(char*, int, char[Blksiz]));

static FILE *tarfile;

static void
usage(void)
{
	eprintf("usage: tar [-f tarfile] [-C dir] [-]x|t\n"
	        "       tar [-f tarfile] [-C dir] [-]c dir\n"
	        "       tar [-C dir] cf tarfile dir\n"
	        "       tar [-C dir] x|tf tarfile\n");
}

int
main(int argc, char *argv[])
{
	char *file = NULL, *dir = ".", *ap;
	char mode = '\0';

	ARGBEGIN {
	case 'x':
	case 'c':
	case 't':
		if(mode)
			usage();
		mode = ARGC();
		break;
	case 'C':
		dir = EARGF(usage());
		break;
	case 'f':
		file = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if(!mode) {
		if(argc < 1)
			usage();

		for(ap = argv[0]; *ap; ap++) {
			switch(*ap) {
			case 'x':
			case 'c':
			case 't':
				if(mode)
					usage();
				mode = *ap;
				break;
			case 'f':
				if(argc < 2)
					usage();
				argc--, argv++;
				file = argv[0];
				break;
			case 'C':
				if(argc < 2)
					usage();
				argc--, argv++;
				dir = argv[0];
				break;
			default:
				usage();
			}
		}
		argc--, argv++;
	}

	if(!mode || argc != (mode == 'c'))
		usage();

	if(file) {
		tarfile = fopen(file, (mode == 'c') ? "wb" : "rb");
		if(!tarfile)
			eprintf("tar: open '%s':", file);
	} else {
		tarfile = (mode == 'c') ? stdout : stdin;
	}

	chdir(dir);

	if(mode == 'c') {
		c(argv[0]);
	} else {
		xt((mode == 'x') ? unarchive : print);
	}

	return 0;
}

void
putoctal(char *dst, unsigned num, int n)
{
	snprintf(dst, n, "%.*o", n-1, num);
}

int
archive(const char* path, const struct stat* sta, int type)
{
	unsigned char b[Blksiz];
	unsigned chksum;
	int l, x;
	Header *h = (void*)b;
	FILE *f = NULL;
	struct stat st;
	struct passwd *pw;
	struct group *gr;
	mode_t mode;

	lstat(path, &st);
	pw = getpwuid(st.st_uid);
	gr = getgrgid(st.st_gid);

	memset(b, 0, sizeof b);
	snprintf(h->name, sizeof h->name, "%s", path);
	putoctal(h->mode,  (unsigned)st.st_mode&0777, sizeof h->mode);
	putoctal(h->uid,   (unsigned)st.st_uid,       sizeof h->uid);
	putoctal(h->gid,   (unsigned)st.st_gid,       sizeof h->gid);
	putoctal(h->size,  0,                         sizeof h->size);
	putoctal(h->mtime, (unsigned)st.st_mtime,     sizeof h->mtime);
	memcpy(h->magic,   "ustar",                   sizeof h->magic);
	memcpy(h->version, "00",                      sizeof h->version);
	snprintf(h->uname, sizeof h->uname, "%s", pw->pw_name);
	snprintf(h->gname, sizeof h->gname, "%s", gr->gr_name);

	mode = st.st_mode;
	if(S_ISREG(mode)) {
		h->type = REG;
		putoctal(h->size, (unsigned)st.st_size,  sizeof h->size);
		f = fopen(path, "r");
	} else if(S_ISDIR(mode)) {
		h->type = DIRECTORY;
	} else if(S_ISLNK(mode)) {
		h->type = SYMLINK;
		readlink(path, h->link, (sizeof h->link)-1);
	} else if(S_ISCHR(mode) || S_ISBLK(mode)) {
		h->type = S_ISCHR(mode) ? CHARDEV : BLOCKDEV;
		putoctal(h->major, (unsigned)major(st.st_dev), sizeof h->major);
		putoctal(h->minor, (unsigned)minor(st.st_dev), sizeof h->minor);
	} else if(S_ISFIFO(mode)) {
		h->type = FIFO;
	}

	memset(h->chksum, ' ', sizeof h->chksum);
	for(x = 0, chksum = 0; x < sizeof *h; x++)
		chksum += b[x];
	putoctal(h->chksum, chksum, sizeof h->chksum);

	fwrite(b, Blksiz, 1, tarfile);
	if(!f)
		return 0;
	while((l = fread(b, 1, Blksiz, f)) > 0) {
		if(l < Blksiz)
			memset(b+l, 0, Blksiz-l);
		fwrite(b, Blksiz, 1, tarfile);
	}
	fclose(f);
	return 0;
}

int
unarchive(char *fname, int l, char b[Blksiz])
{
	char lname[101];
	FILE *f = NULL;
	unsigned long  mode, major, minor, type;
	Header *h = (void*)b;

	unlink(fname);
	switch(h->type) {
	case REG:
		mode = strtoul(h->mode, 0, 8);
		if(!(f = fopen(fname, "w")) || chmod(fname, mode))
			perror(fname);
		break;
	case HARDLINK:
	case SYMLINK:
		snprintf(lname, sizeof lname, "%s", h->link);
		if(!((h->type == HARDLINK) ? link : symlink)(lname, fname))
			perror(fname);
		break;
	case DIRECTORY:
		mode = strtoul(h->mode, 0, 8);
		if(mkdir(fname, (mode_t)mode))
			perror(fname);
		break;
	case CHARDEV:
	case BLOCKDEV:
		mode = strtoul(h->mode, 0, 8);
		major = strtoul(h->major, 0, 8);
		minor = strtoul(h->mode, 0, 8);
		type = (h->type == CHARDEV) ? S_IFCHR : S_IFBLK;
		if(mknod(fname, type | mode, makedev(major, minor)))
			perror(fname);
		break;
	case FIFO:
		mode = strtoul(h->mode, 0, 8);
		if(mknod(fname, S_IFIFO | mode, 0))
			perror(fname);
		break;
	default:
		fprintf(stderr, "usupported tarfiletype %c\n", h->type);
	}
	if(getuid() == 0 && chown(fname, strtoul(h->uid, 0, 8),
	                                 strtoul(h->gid, 0, 8)))
		perror(fname);

	for(; l > 0; l -= Blksiz) {
		fread(b, Blksiz, 1, tarfile);
		if(f)
			fwrite(b, MIN(l, 512), 1, f);
	}
	if(f)
		fclose(f);
	return 0;
}

int
print(char * fname, int l, char b[Blksiz])
{
	puts(fname);
	for(; l > 0; l -= Blksiz)
		fread(b, Blksiz, 1, tarfile);
	return 0;
}

void
c(char * dir)
{
	ftw(dir, archive, FOPEN_MAX);
}

void
xt(int (*fn)(char*, int, char[Blksiz]))
{
	char b[Blksiz], fname[101];
	Header *h = (void*)b;

	while(fread(b, Blksiz, 1, tarfile) && h->name[0] != '\0') {
		snprintf(fname, sizeof fname, "%s", h->name);
		fn(fname, strtol(h->size, 0, 8), b);
	}
}
