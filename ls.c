/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

typedef struct {
	char *name;
	mode_t mode;
	nlink_t nlink;
	uid_t uid;
	gid_t gid;
	off_t size;
	time_t mtime;
} Entry;

static int entcmp(const void *, const void *);
static void ls(Entry *);
static void lsdir(const char *);
static void mkent(Entry *, char *, bool);
static void output(Entry *);

static bool aflag = false;
static bool dflag = false;
static bool lflag = false;
static bool tflag = false;
static bool Uflag = false;
static bool first = true;
static bool many;

static void
usage(void)
{
	eprintf("usage: %s [-adltU] [FILE...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i;
	Entry *ents;

	ARGBEGIN {
	case 'a':
		aflag = true;
		break;
	case 'd':
		dflag = true;
		break;
	case 'l':
		lflag = true;
		break;
	case 't':
		tflag = true;
		break;
	case 'U':
		Uflag = true;
		break;
	default:
		usage();
	} ARGEND;

	many = (argc > 1);
	if(argc == 0)
		*--argv = ".", argc++;

	if(!(ents = malloc(argc * sizeof *ents)))
		eprintf("malloc:");
	for(i = 0; i < argc; i++)
		mkent(&ents[i], argv[i], true);
	qsort(ents, argc, sizeof *ents, entcmp);
	for(i = 0; i < argc; i++)
		ls(&ents[i]);

	return 0;
}

int
entcmp(const void *va, const void *vb)
{
	const Entry *a = va, *b = vb;

	if(tflag)
		return b->mtime - a->mtime;
	else
		return strcmp(a->name, b->name);
}

void
ls(Entry *ent)
{
	if(S_ISDIR(ent->mode) && !dflag) {
		lsdir(ent->name);
	} else {
		output(ent);
	}
}

void
lsdir(const char *path)
{
	char *cwd, *p;
	long i, n = 0;
	struct dirent *d;
	DIR *dp;
	Entry ent, *ents = NULL;
	size_t sz;

	cwd = agetcwd();
	if(!(dp = opendir(path)))
		eprintf("opendir %s:", path);
	if(chdir(path) == -1)
		eprintf("chdir %s:", path);

	if(many) {
		if(!first)
			putchar('\n');
		printf("%s:\n", path);
		first = false;
	}

	while((d = readdir(dp))) {
		if(d->d_name[0] == '.' && !aflag)
			continue;
		if(Uflag){
			mkent(&ent, d->d_name, lflag);
			output(&ent);
		} else {
			if(!(ents = realloc(ents, ++n * sizeof *ents)))
				eprintf("realloc:");
			if(!(p = malloc((sz = strlen(d->d_name)+1))))
				eprintf("malloc:");
			memcpy(p, d->d_name, sz);
			mkent(&ents[n-1], p, tflag || lflag);
		}
	}
	closedir(dp);
	if(!Uflag){
		qsort(ents, n, sizeof *ents, entcmp);
		for(i = 0; i < n; i++) {
			output(&ents[i]);
			free(ents[i].name);
		}
	}
	if(chdir(cwd) == -1)
		eprintf("chdir %s:", cwd);
	free(ents);
	free(cwd);
}

void
mkent(Entry *ent, char *path, bool dostat)
{
	struct stat st;

	ent->name   = path;
	if(!dostat)
		return;
	if(lstat(path, &st) == -1)
		eprintf("lstat %s:", path);
	ent->mode   = st.st_mode;
	ent->nlink  = st.st_nlink;
	ent->uid    = st.st_uid;
	ent->gid    = st.st_gid;
	ent->size   = st.st_size;
	ent->mtime  = st.st_mtime;
}

void
output(Entry *ent)
{
	char buf[BUFSIZ], *fmt;
	char mode[] = "----------";
	ssize_t len;
	struct group *gr;
	struct passwd *pw;

	if(!lflag) {
		puts(ent->name);
		return;
	}
	if(S_ISREG(ent->mode))
		mode[0] = '-';
	else if(S_ISBLK(ent->mode))
		mode[0] = 'b';
	else if(S_ISCHR(ent->mode))
		mode[0] = 'c';
	else if(S_ISDIR(ent->mode))
		mode[0] = 'd';
	else if(S_ISFIFO(ent->mode))
		mode[0] = 'p';
	else if(S_ISLNK(ent->mode))
		mode[0] = 'l';
	else if(S_ISSOCK(ent->mode))
		mode[0] = 's';
	else
		mode[0] = '?';

	if(ent->mode & S_IRUSR) mode[1] = 'r';
	if(ent->mode & S_IWUSR) mode[2] = 'w';
	if(ent->mode & S_IXUSR) mode[3] = 'x';
	if(ent->mode & S_IRGRP) mode[4] = 'r';
	if(ent->mode & S_IWGRP) mode[5] = 'w';
	if(ent->mode & S_IXGRP) mode[6] = 'x';
	if(ent->mode & S_IROTH) mode[7] = 'r';
	if(ent->mode & S_IWOTH) mode[8] = 'w';
	if(ent->mode & S_IXOTH) mode[9] = 'x';

	if(ent->mode & S_ISUID) mode[3] = (mode[3] == 'x') ? 's' : 'S';
	if(ent->mode & S_ISGID) mode[6] = (mode[6] == 'x') ? 's' : 'S';

	errno = 0;
	pw = getpwuid(ent->uid);
	if(errno)
		eprintf("getpwuid %d:", ent->uid);
	else if(!pw)
		eprintf("getpwuid %d: no such user\n", ent->uid);

	errno = 0;
	gr = getgrgid(ent->gid);
	if(errno)
		eprintf("getgrgid %d:", ent->gid);
	else if(!gr)
		eprintf("getgrgid %d: no such group\n", ent->gid);

	if(time(NULL) > ent->mtime + (180*24*60*60)) /* 6 months ago? */
		fmt = "%b %d  %Y";
	else
		fmt = "%b %d %H:%M";

	strftime(buf, sizeof buf, fmt, localtime(&ent->mtime));
	printf("%s %2ld %-4s %-5s %6lu %s %s", mode, (long)ent->nlink, pw->pw_name,
	       gr->gr_name, (unsigned long)ent->size, buf, ent->name);
	if(S_ISLNK(ent->mode)) {
		if((len = readlink(ent->name, buf, sizeof buf)) == -1)
			eprintf("readlink %s:", ent->name);
		buf[len] = '\0';
		printf(" -> %s", buf);
	}
	putchar('\n');
}

