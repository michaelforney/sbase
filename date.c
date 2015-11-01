/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-u] [-d time] [+format]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct tm *now;
	struct tm *(*tztime)(const time_t *) = localtime;
	time_t t;
	char buf[BUFSIZ], *fmt = "%c", *tz = "local";

	t = time(NULL);

	ARGBEGIN {
	case 'd':
		t = estrtonum(EARGF(usage()), 0, LLONG_MAX);
		break;
	case 'u':
		tztime = gmtime;
		tz = "gm";
		break;
	default:
		usage();
	} ARGEND

	if (argc) {
		if (argc != 1 || argv[0][0] != '+')
			usage();
		else
			fmt = &argv[0][1];
	}
	if (!(now = tztime(&t)))
		eprintf("%stime failed\n", tz);

	strftime(buf, sizeof(buf), fmt, now);
	puts(buf);

	return fshut(stdout, "<stdout>");
}
