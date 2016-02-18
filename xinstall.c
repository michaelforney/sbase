/* See LICENSE file for copyright and license details. */
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "util.h"
#include "text.h"

static int Dflag = 0;
static int sflag = 0;
static gid_t group;
static uid_t owner;
static mode_t mode = 0755;

static void
make_dir(char *dir, int was_missing)
{
	if (!mkdir(dir, was_missing ? 0755 : mode)) {
		if (!was_missing && (lchown(dir, owner, group) < 0))
			eprintf("lchmod %s:", dir);
	} else if (errno != EEXIST) {
		eprintf("mkdir %s:", dir);
	}
}

static void
make_dirs(char *dir, int was_missing)
{
	char *p;
	for (p = strchr(dir + (dir[0] == '/'), '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';
		make_dir(dir, was_missing);
		*p = '/';
	}
	make_dir(dir, was_missing);
}

static void
strip(const char *filename)
{
	pid_t pid = fork();
	switch (pid) {
	case -1:
		eprintf("fork:");
	case 0:
		execlp("strip", "strip", "--", filename, (char *)0);
		eprintf("exec: strip:");
	default:
		waitpid(pid, NULL, 0);
		break;
	}
}

static int
install(const char *s1, const char *s2, int depth)
{
	DIR *dp;
	FILE *f1, *f2;
	struct dirent *d;
	struct stat st;
	ssize_t r;
	char target[PATH_MAX], ns1[PATH_MAX], ns2[PATH_MAX];

	if (stat(s1, &st) < 0)
		eprintf("stat %s:", s1);

	if (S_ISLNK(st.st_mode)) {
		if ((r = readlink(s1, target, sizeof(target) - 1)) >= 0) {
			target[r] = '\0';
			if (unlink(s2) < 0 && errno != ENOENT)
				eprintf("unlink %s:", s2);
			else if (symlink(target, s2) < 0)
				eprintf("symlink %s -> %s:", s2, target);
		}
	} else if (S_ISDIR(st.st_mode)) {
		if (!(dp = opendir(s1)))
			eprintf("opendir %s:", s1);
		if (mkdir(s2, mode | 0111) < 0 && errno != EEXIST)
			eprintf("mkdir %s:", s2);

		while ((d = readdir(dp))) {
			if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
				continue;

			estrlcpy(ns1, s1, sizeof(ns1));
			if (s1[strlen(s1) - 1] != '/')
				estrlcat(ns1, "/", sizeof(ns1));
			estrlcat(ns1, d->d_name, sizeof(ns1));

			estrlcpy(ns2, s2, sizeof(ns2));
			if (s2[strlen(s2) - 1] != '/')
				estrlcat(ns2, "/", sizeof(ns2));
			estrlcat(ns2, d->d_name, sizeof(ns2));

			fnck(ns1, ns2, install, depth + 1);
		}

		closedir(dp);
	} else if (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode) ||
	           S_ISSOCK(st.st_mode) || S_ISFIFO(st.st_mode)) {
		if (unlink(s2) < 0 && errno != ENOENT)
			eprintf("unlink %s:", s2);
		else if (mknod(s2, (st.st_mode & ~07777) | mode, st.st_rdev) < 0)
			eprintf("mknod %s:", s2);
	} else {
		if (!(f1 = fopen(s1, "r")))
			eprintf("fopen %s:", s1);
		if (!(f2 = fopen(s2, "w"))) {
			if (unlink(s2) < 0 && errno != ENOENT)
				eprintf("unlink %s:", s2);
			else if (!(f2 = fopen(s2, "w")))
				eprintf("fopen %s:", s2);
		}
		concat(f1, s1, f2, s2);

		fchmod(fileno(f2), mode);

		if (fclose(f2) == EOF)
			eprintf("fclose %s:", s2);
		if (fclose(f1) == EOF)
			eprintf("fclose %s:", s1);

		if (sflag)
			strip(s2);
	}

	if (lchown(s2, owner, group) < 0)
		eprintf("lchown %s:", s2);

	return 0;
}

static void
usage(void)
{
	eprintf("usage: %s [-g group] [-o owner] [-m mode] (-d dir ... | [-Ds] (-t dest source ... | source ... dest))\n", argv0);
}

int
main(int argc, char *argv[])
{
	int dflag = 0;
	char *gflag = 0;
	char *oflag = 0;
	char *mflag = 0;
	char *tflag = 0;
	struct group *gr;
	struct passwd *pw;
	struct stat st;
	char *p;

	ARGBEGIN {
	case 'd':
		dflag = 1;
		break;
	case 'D':
		Dflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'g':
		gflag = EARGF(usage());
		break;
	case 'o':
		oflag = EARGF(usage());
		break;
	case 'm':
		mflag = EARGF(usage());
		break;
	case 't':
		tflag = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND

	if (argc < 1 + (!tflag & !dflag) || dflag & (Dflag | sflag | !!tflag))
		usage();

	if (gflag) {
		errno = 0;
		gr = getgrnam(gflag);
		if (gr) {
			group = gr->gr_gid;
		} else {
			if (errno)
				eprintf("getgrnam %s:", gflag);
			group = estrtonum(gflag, 0, UINT_MAX);
		}
	} else {
		group = getgid();
	}

	if (oflag) {
		errno = 0;
		pw = getpwnam(oflag);
		if (pw) {
			owner = pw->pw_uid;
		} else {
			if (errno)
				eprintf("getpwnam %s:", oflag);
			owner = estrtonum(oflag, 0, UINT_MAX);
		}
	} else {
		owner = getuid();
	}

	if (mflag) {
		mode = parsemode(mflag, mode, 0);
		if (mode < 0)
			return 1;
	}

	if (tflag) {
		memmove(argv - 1, argv, argc);
		argv[argc++] = tflag;
	}
	if (tflag || argc > 2) {
		if (stat(argv[argc - 1], &st) < 0) {
			if ((errno == ENOENT) && Dflag) {
				make_dirs(argv[argc - 1], 1);
			} else {
				eprintf("stat %s:", argv[argc - 1]);
			}
		} else if (!S_ISDIR(st.st_mode)) {
			eprintf("%s: not a directory\n", argv[argc - 1]);
		}
	}

	if (dflag) {
		for (; *argv; argc--, argv++)
			make_dirs(*argv, 0);
	} else {
		if (stat(argv[argc - 1], &st) < 0) {
			if (errno != ENOENT)
				eprintf("stat %s:", argv[argc - 1]);
			if (tflag || Dflag || argc > 2) {
				if ((p = strrchr(argv[argc - 1], '/')) != NULL) {
					*p = '\0';
					make_dirs(argv[argc - 1], 1);
					*p = '/';
				}
			}
		}
		enmasse(argc, argv, install);
	}

	return 0;
}
