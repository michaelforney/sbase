/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "util.h"

static void user(struct passwd *pw);
static void curproc(void);

static void
usage(void)
{
	eprintf("usage: %s [user]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct passwd *pw;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	switch (argc) {
	case 0:
		curproc();
		break;
	case 1:
		errno = 0;
		pw = getpwnam(argv[0]);
		if (errno != 0)
			eprintf("getpwnam %s:", argv[0]);
		else if (!pw)
			eprintf("getpwnam %s: no such user\n", argv[0]);
		user(pw);
		break;
	default:
		usage();
	}

	return EXIT_SUCCESS;
}

static void
user(struct passwd *pw)
{
	struct group *gr;
	gid_t gid, groups[NGROUPS_MAX];
	int ngroups;
	int i;

	printf("uid=%u(%s)", pw->pw_uid, pw->pw_name);
	printf(" gid=%u", pw->pw_gid);
	if (!(gr = getgrgid(pw->pw_gid)))
		eprintf("getgrgid:");
	printf("(%s)", gr->gr_name);

	ngroups = NGROUPS_MAX;
	getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);
	for (i = 0; i < ngroups; i++) {
		gid = groups[i];
		printf("%s%u", !i ? " groups=" : ",", gid);
		if (!(gr = getgrgid(gid)))
			eprintf("getgrgid:");
		printf("(%s)", gr->gr_name);
	}
	putchar('\n');
}

static void
curproc(void)
{
	struct passwd *pw;
	struct group *gr;
	uid_t uid, euid;
	gid_t gid, egid, groups[NGROUPS_MAX];
	int ngroups;
	int i;

	/* Print uid/euid info */
	uid = getuid();
	printf("uid=%u", uid);
	if (!(pw = getpwuid(uid)))
		eprintf("getpwuid:");
	printf("(%s)", pw->pw_name);
	if ((euid = geteuid()) != uid) {
		printf(" euid=%u", euid);
		if (!(pw = getpwuid(euid)))
			eprintf("getpwuid:");
		printf("(%s)", pw->pw_name);
	}

	/* Print gid/egid info */
	gid = getgid();
	printf(" gid=%u", gid);
	if (!(gr = getgrgid(gid)))
		eprintf("getgrgid:");
	printf("(%s)", gr->gr_name);
	if ((egid = getegid()) != gid) {
		printf(" egid=%u", egid);
		if (!(gr = getgrgid(egid)))
			eprintf("getgrgid:");
		printf("(%s)", gr->gr_name);
	}

	/* Print groups */
	ngroups = getgroups(NGROUPS_MAX, groups);
	if (ngroups < 0)
		eprintf("getgroups:");
	for (i = 0; i < ngroups; i++) {
		gid = groups[i];
		printf("%s%u", !i ? " groups=" : ",", gid);
		if (!(gr = getgrgid(gid)))
			eprintf("getgrgid:");
		printf("(%s)", gr->gr_name);
	}
	putchar('\n');
}
