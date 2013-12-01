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

	errno = 0;
	switch (argc) {
	case 0:
		pw = getpwuid(getuid());
		if (errno != 0)
			eprintf("getpwuid %d:", getuid());
		else if (!pw)
			eprintf("getpwuid %d: no such user\n", getuid());
		user(pw);
		break;
	case 1:
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
