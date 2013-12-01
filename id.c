/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "util.h"

static void curproc(void);

static void
usage(void)
{
	eprintf("usage: %s\n", argv0);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	curproc();

	return EXIT_SUCCESS;
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
