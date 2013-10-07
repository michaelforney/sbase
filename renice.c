/* See LICENSE file for copyright and license details. */
#include <sys/resource.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static int strtop(const char *);
static bool renice(int, int, long);
static void usage(void);

int
main(int argc, char **argv)
{
	const char *adj = NULL;
	long val;
	int i, which = PRIO_PROCESS, status = EXIT_SUCCESS;

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

	if(argc == 0 || !adj)
		usage();

	val = estrtol(adj, 10);
	for(i = 0; i < argc; i++) {
		int who = -1;

		if(which == PRIO_USER) {
			const struct passwd *pwd;

			errno = 0;
			do pwd = getpwnam(argv[i]); while(errno == EINTR);

			if(pwd)
				who = pwd->pw_uid;
			else if(errno != 0) {
				perror("can't read passwd");
				status = EXIT_FAILURE;
				continue;
			}
		}
		if(who < 0)
			who = strtop(argv[i]);

		if(who < 0 || !renice(which, who, val))
			status = EXIT_FAILURE;
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
	if(*end != '\0') {
		fprintf(stderr, "%s: not an integer\n", s);
		return -1;
	}
	if(errno != 0 || n <= 0 || n > INT_MAX) {
		fprintf(stderr, "%s: invalid value\n", s);
		return -1;
	}

	return (int)n;
}

static bool
renice(int which, int who, long adj)
{
	errno = 0;
	adj += getpriority(which, who);
	if(errno != 0) {
		fprintf(stderr, "can't get %d nice level: %s\n",
		        who, strerror(errno));
		return false;
	}

	/* PRIO_{MIN,MAX} does not exist in musl libc */
	adj = MAX(-20, MIN(adj, 20));
	if(setpriority(which, who, (int)adj) == -1) {
		fprintf(stderr, "can't set %d nice level: %s\n",
		        who, strerror(errno));
		return false;
	}

	return true;
}

static void
usage(void)
{
	eprintf("renice -n inc [-g | -p | -u] ID ...\n");
}
