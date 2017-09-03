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
	time_t t;
	char buf[BUFSIZ], *fmt = "%c";

	t = time(NULL);

	ARGBEGIN {
	case 'd':
		t = estrtonum(EARGF(usage()), 0, LLONG_MAX);
		break;
	case 'u':
		if (setenv("TZ", "UTC0", 1) < 0)
			eprintf("setenv:");
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
	if (!(now = localtime(&t)))
		eprintf("localtime:");

	strftime(buf, sizeof(buf), fmt, now);
	puts(buf);

	return fshut(stdout, "<stdout>");
}
