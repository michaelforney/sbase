/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#include "util.h"

static int strtop(const char *);
static int renice(int, int, long);

static void
usage(void)
{
	eprintf("renice -n inc [-g | -p | -u] ID ...\n");
}

int
main(int argc, char *argv[])
{
	const char *adj = NULL;
	long val;
	int i, which = PRIO_PROCESS, status = 0;
	struct passwd *pw;
	int who;

	ARGBEGIN {
	case 'n':
		adj = EARGF(usage());
		break;
	case 'g':
		which = PRIO_PGRP;
		break;
	case 'p':
		which = PRIO_PROCESS;
		break;
	case 'u':
		which = PRIO_USER;
		break;
	default:
		usage();
		break;
	} ARGEND;

	if (argc == 0 || !adj)
		usage();

	val = estrtol(adj, 10);
	for (i = 0; i < argc; i++) {
		who = -1;
		if (which == PRIO_USER) {
			errno = 0;
			pw = getpwnam(argv[i]);
			if (!pw) {
				if (errno != 0)
					weprintf("getpwnam %s:", argv[i]);
				else
					weprintf("getpwnam %s: no user found\n", argv[i]);
				status = 1;
				continue;
			}
			who = pw->pw_uid;
		}
		if (who < 0)
			who = strtop(argv[i]);

		if (who < 0 || !renice(which, who, val))
			status = 1;
	}

	return status;
}

static int
strtop(const char *s)
{
	char *end;
	long n;

	errno = 0;
	n = strtol(s, &end, 10);
	if (*end != '\0') {
		weprintf("%s: not an integer\n", s);
		return -1;
	}
	if (errno != 0 || n <= 0 || n > INT_MAX) {
		weprintf("%s: invalid value\n", s);
		return -1;
	}

	return (int)n;
}

static int
renice(int which, int who, long adj)
{
	errno = 0;
	adj += getpriority(which, who);
	if (errno != 0) {
		weprintf("getpriority %d:", who);
		return 0;
	}

	adj = MAX(PRIO_MIN, MIN(adj, PRIO_MAX));
	if (setpriority(which, who, (int)adj) < 0) {
		weprintf("setpriority %d:", who);
		return 0;
	}

	return 1;
}
