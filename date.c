/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	char buf[BUFSIZ], c;
	char *fmt = "%c";
	struct tm *now = NULL;
	struct tm *(*tztime)(const time_t *) = localtime;
	const char *tz = "local";
	time_t t;

	t = time(NULL);
	while((c = getopt(argc, argv, "d:u")) != -1)
		switch(c) {
		case 'd':
			t = estrtol(optarg, 0);
			break;
		case 'u':
			tztime = gmtime;
			tz = "gm";
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind < argc && argv[optind][0] == '+')
		fmt = &argv[optind][1];
	if(!(now = tztime(&t)))
		eprintf("%stime failed\n", tz);

	strftime(buf, sizeof buf, fmt, now);
	puts(buf);
	return EXIT_SUCCESS;
}
