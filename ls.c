/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "utf.h"
#include "util.h"

struct entry {
	char   *name;
	mode_t  mode, tmode;
	nlink_t nlink;
	uid_t   uid;
	gid_t   gid;
	off_t   size;
	time_t  t;
	ino_t   ino;
};

static int Aflag = 0;
static int aflag = 0;
static int cflag = 0;
static int dflag = 0;
static int Fflag = 0;
static int fflag = 0;
static int Hflag = 0;
static int hflag = 0;
static int iflag = 0;
static int Lflag = 0;
static int lflag = 0;
static int nflag = 0;
static int pflag = 0;
static int qflag = 0;
static int Rflag = 0;
static int Sflag = 0;
static int rflag = 0;
static int tflag = 0;
static int Uflag = 0;
static int uflag = 0;
static int first = 1;
static int many;

static void ls(const struct entry *ent, int recurse);

static void
mkent(struct entry *ent, char *path, int dostat, int follow)
{
	struct stat st;

	ent->name = path;
	if (!dostat)
		return;
	if ((follow ? stat : lstat)(path, &st) < 0)
		eprintf("%s %s:", follow ? "stat" : "lstat", path);
	ent->mode  = st.st_mode;
	ent->nlink = st.st_nlink;
	ent->uid   = st.st_uid;
	ent->gid   = st.st_gid;
	ent->size  = st.st_size;
	if (cflag)
		ent->t = st.st_ctime;
	else if (uflag)
		ent->t = st.st_atime;
	else
		ent->t = st.st_mtime;
	ent->ino   = st.st_ino;
	if (S_ISLNK(ent->mode))
		ent->tmode = stat(path, &st) == 0 ? st.st_mode : 0;
}

static char *
indicator(mode_t mode)
{
	if (pflag || Fflag)
		if (S_ISDIR(mode))
			return "/";

	if (Fflag) {
		if (S_ISLNK(mode))
			return "@";
		else if (S_ISFIFO(mode))
			return "|";
		else if (S_ISSOCK(mode))
			return "=";
		else if (mode & S_IXUSR || mode & S_IXGRP || mode & S_IXOTH)
			return "*";
	}

	return "";
}

static void
output(const struct entry *ent)
{
	struct group *gr;
	struct passwd *pw;
	ssize_t len;
	char buf[BUFSIZ], *fmt,
	     pwname[_SC_LOGIN_NAME_MAX], grname[_SC_LOGIN_NAME_MAX],
	     mode[] = "----------";

	if (iflag)
		printf("%lu ", (unsigned long)ent->ino);
	if (!lflag) {
		printf("%s%s\n", ent->name, indicator(ent->mode));
		return;
	}
	if (S_ISREG(ent->mode))
		mode[0] = '-';
	else if (S_ISBLK(ent->mode))
		mode[0] = 'b';
	else if (S_ISCHR(ent->mode))
		mode[0] = 'c';
	else if (S_ISDIR(ent->mode))
		mode[0] = 'd';
	else if (S_ISFIFO(ent->mode))
		mode[0] = 'p';
	else if (S_ISLNK(ent->mode))
		mode[0] = 'l';
	else if (S_ISSOCK(ent->mode))
		mode[0] = 's';
	else
		mode[0] = '?';

	if (ent->mode & S_IRUSR) mode[1] = 'r';
	if (ent->mode & S_IWUSR) mode[2] = 'w';
	if (ent->mode & S_IXUSR) mode[3] = 'x';
	if (ent->mode & S_IRGRP) mode[4] = 'r';
	if (ent->mode & S_IWGRP) mode[5] = 'w';
	if (ent->mode & S_IXGRP) mode[6] = 'x';
	if (ent->mode & S_IROTH) mode[7] = 'r';
	if (ent->mode & S_IWOTH) mode[8] = 'w';
	if (ent->mode & S_IXOTH) mode[9] = 'x';

	if (ent->mode & S_ISUID) mode[3] = (mode[3] == 'x') ? 's' : 'S';
	if (ent->mode & S_ISGID) mode[6] = (mode[6] == 'x') ? 's' : 'S';
	if (ent->mode & S_ISVTX) mode[9] = (mode[9] == 'x') ? 't' : 'T';

	if (!nflag && (pw = getpwuid(ent->uid)))
		snprintf(pwname, LEN(pwname), "%s", pw->pw_name);
	else
		snprintf(pwname, LEN(pwname), "%d", ent->uid);

	if (!nflag && (gr = getgrgid(ent->gid)))
		snprintf(grname, LEN(grname), "%s", gr->gr_name);
	else
		snprintf(grname, LEN(grname), "%d", ent->gid);

	if (time(NULL) > ent->t + (180 * 24 * 60 * 60)) /* 6 months ago? */
		fmt = "%b %d  %Y";
	else
		fmt = "%b %d %H:%M";

	strftime(buf, sizeof(buf), fmt, localtime(&ent->t));
	printf("%s %4ld %-8.8s %-8.8s ", mode, (long)ent->nlink, pwname, grname);

	if (hflag)
		printf("%10s ", humansize(ent->size));
	else
		printf("%10lu ", (unsigned long)ent->size);
	printf("%s %s%s", buf, ent->name, indicator(ent->mode));
	if (S_ISLNK(ent->mode)) {
		if ((len = readlink(ent->name, buf, sizeof(buf) - 1)) < 0)
			eprintf("readlink %s:", ent->name);
		buf[len] = '\0';
		printf(" -> %s%s", buf, indicator(ent->tmode));
	}
	putchar('\n');
}

static int
entcmp(const void *va, const void *vb)
{
	int cmp = 0;
	const struct entry *a = va, *b = vb;

	if (Sflag)
		cmp = b->size - a->size;
	else if (tflag)
		cmp = b->t - a->t;

	return cmp ? cmp : strcmp(a->name, b->name);
}

static void
lsdir(const char *path)
{
	DIR *dp;
	Rune r;
	struct entry ent, *ents = NULL;
	struct dirent *d;
	size_t i, n = 0, len;
	char cwd[PATH_MAX], *p, *q, *name;

	if (!getcwd(cwd, sizeof(cwd)))
		eprintf("getcwd:");
	if (!(dp = opendir(path)))
		eprintf("opendir %s:", path);
	if (chdir(path) < 0)
		eprintf("chdir %s:", path);

	if (many || Rflag) {
		if (!first)
			putchar('\n');
		printf("%s:\n", path);
	}
	first = 0;

	while ((d = readdir(dp))) {
		if (d->d_name[0] == '.' && !aflag && !Aflag)
			continue;
		else if (Aflag)
			if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
				continue;
		if (Uflag){
			mkent(&ent, d->d_name, Fflag || lflag || pflag || iflag || Rflag, Lflag);
			ls(&ent, Rflag);
		} else {
			ents = ereallocarray(ents, ++n, sizeof(*ents));
			name = p = estrdup(d->d_name);
			if (qflag) {
				q = d->d_name;
				while (*q) {
					len = chartorune(&r, q);
					if (isprintrune(r)) {
						memcpy(p, q, len);
						p += len, q += len;
					} else {
						*p++ = '?';
						q += len;
					}
				}
				*p = '\0';
			}
			mkent(&ents[n - 1], name, tflag || Sflag || Fflag || iflag || lflag || pflag || Rflag, Lflag);
		}
	}
	closedir(dp);
	if (!Uflag){
		qsort(ents, n, sizeof(*ents), entcmp);
		for (i = 0; i < n; i++) {
			ls(&ents[rflag ? (n - i - 1) : i], Rflag);
			free(ents[rflag ? (n - i - 1) : i].name);
		}
	}
	if (chdir(cwd) < 0)
		eprintf("chdir %s:", cwd);
	free(ents);
}

static void
ls(const struct entry *ent, int recurse)
{
	if (recurse && (S_ISDIR(ent->mode) || (S_ISLNK(ent->mode) &&
	     S_ISDIR(ent->tmode) && !Fflag && !lflag)) && !dflag)
		lsdir(ent->name);
	else
		output(ent);
}

static void
usage(void)
{
	eprintf("usage: %s [-1AacdFHhiLlqRrtUu] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct entry *ents;
	size_t i;

	ARGBEGIN {
	case '1':
		/* ignore */
		break;
	case 'A':
		Aflag = 1;
		break;
	case 'a':
		aflag = 1;
		break;
	case 'c':
		cflag = 1;
		uflag = 0;
		break;
	case 'd':
		dflag = 1;
		break;
	case 'f':
		aflag = 1;
		fflag = 1;
		Uflag = 1;
		break;
	case 'F':
		Fflag = 1;
		break;
	case 'H':
		Hflag = 1;
		break;
	case 'h':
		hflag = 1;
		break;
	case 'i':
		iflag = 1;
		break;
	case 'L':
		Lflag = 1;
		break;
	case 'l':
		lflag = 1;
		break;
	case 'n':
		lflag = 1;
		nflag = 1;
		break;
	case 'p':
		pflag = 1;
		break;
	case 'q':
		qflag = 1;
		break;
	case 'R':
		Rflag = 1;
		break;
	case 'r':
		if (!fflag)
			rflag = 1;
		break;
	case 'S':
		Sflag = 1;
		tflag = 0;
		break;
	case 't':
		Sflag = 0;
		tflag = 1;
		break;
	case 'U':
		Uflag = 1;
		break;
	case 'u':
		uflag = 1;
		cflag = 0;
		break;
	default:
		usage();
	} ARGEND;

	many = (argc > 1);
	if (argc == 0)
		*--argv = ".", argc++;

	ents = ereallocarray(NULL, argc, sizeof(*ents));

	for (i = 0; i < argc; i++)
		mkent(&ents[i], argv[i], 1, Hflag || Lflag);
	qsort(ents, argc, sizeof(*ents), entcmp);
	for (i = 0; i < argc; i++)
		ls(&ents[rflag ? argc-i-1 : i], 1);

	return fshut(stdout, "<stdout>");
}
