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

static int entcmp(Entry *, Entry *);
static void ls(char *);
static void lsdir(const char *);
static void mkent(Entry *, char *);
static void output(Entry *);

static bool aflag = false;
static bool dflag = false;
static bool lflag = false;
static bool tflag = false;
static bool many;

int
main(int argc, char *argv[])
{
	char c;

	while((c = getopt(argc, argv, "adlt")) != -1)
		switch(c) {
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
		default:
			exit(EXIT_FAILURE);
		}
	many = (argc > optind+1);

	if(optind == argc)
		ls(".");
	else for(; optind < argc; optind++)
		ls(argv[optind]);
	return EXIT_SUCCESS;
}

int
entcmp(Entry *a, Entry *b)
{
	if(tflag)
		return b->mtime - a->mtime;
	else
		return strcmp(a->name, b->name);
}

void
ls(char *path)
{
	Entry ent;

	mkent(&ent, path);
	if(S_ISDIR(ent.mode) && !dflag)
		lsdir(path);
	else
		output(&ent);
}

void
lsdir(const char *path)
{
	char *cwd, *p;
	long i, n = 0;
	struct dirent *d;
	DIR *dp;
	Entry *ents = NULL;

	cwd = agetcwd();
	if(!(dp = opendir(path)))
		eprintf("opendir %s:", path);
	if(chdir(path) != 0)
		eprintf("chdir %s:", path);

	while((d = readdir(dp))) {
		if(d->d_name[0] == '.' && !aflag)
			continue;
		if(!(ents = realloc(ents, ++n * sizeof *ents)))
			eprintf("realloc:");
		if(!(p = strdup(d->d_name)))
			eprintf("strdup:");
		mkent(&ents[n-1], p);
	}
	closedir(dp);
	qsort(ents, n, sizeof *ents, (int (*)(const void *, const void *))entcmp);

	if(many)
		printf("%s:\n", path);
	for(i = 0; i < n; i++) {
		output(&ents[i]);
		free(ents[i].name);
	}
	if(chdir(cwd) != 0)
		eprintf("chdir %s:", cwd);
	free(ents);
	free(cwd);
}

void
mkent(Entry *ent, char *path)
{
	struct stat st;

	if(lstat(path, &st) != 0)
		eprintf("lstat %s:", path);
	ent->name   = path;
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
	char buf[BUFSIZ], mode[11], *fmt;
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

	mode[1] = (ent->mode & S_IRUSR) ? 'r' : '-';
	mode[2] = (ent->mode & S_IWUSR) ? 'w' : '-';
	if(ent->mode & S_ISUID)
		mode[3] = (ent->mode & S_IXUSR) ? 's' : 'S';
	else
		mode[3] = (ent->mode & S_IXUSR) ? 'x' : '-';

	mode[4] = (ent->mode & S_IRGRP) ? 'r' : '-';
	mode[5] = (ent->mode & S_IWGRP) ? 'w' : '-';
	if(ent->mode & S_ISGID)
		mode[6] = (ent->mode & S_IXGRP) ? 's' : 'S';
	else
		mode[6] = (ent->mode & S_IXGRP) ? 'x' : '-';

	mode[7] = (ent->mode & S_IROTH) ? 'r' : '-';
	mode[8] = (ent->mode & S_IWOTH) ? 'w' : '-';
	mode[9] = (ent->mode & S_IXOTH) ? 'x' : '-';
	mode[10] = '\0';

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
		fmt = "%b %d %Y";
	else
		fmt = "%b %d %H:%M";

	strftime(buf, sizeof buf, fmt, localtime(&ent->mtime));
	printf("%s %2d %s %s %6lu %s %s\n", mode, ent->nlink, pw->pw_name,
	       gr->gr_name, (unsigned long)ent->size, buf, ent->name);
}
