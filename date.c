/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-u] [-d format] [+FORMAT]\n", argv0);
}

int
main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	char *fmt = "%c";
	struct tm *now = NULL;
	struct tm *(*tztime)(const time_t *) = localtime;
	const char *tz = "local";
	time_t t;

	t = time(NULL);
	ARGBEGIN {
	case 'd':
		t = estrtol(EARGF(usage()), 0);
		break;
	case 'u':
		tztime = gmtime;
		tz = "gm";
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 0 && argv[0][0] == '+')
		fmt = &argv[0][1];
	if (!(now = tztime(&t)))
		eprintf("%stime failed\n", tz);

	strftime(buf, sizeof(buf), fmt, now);
	puts(buf);

	return 0;
}
