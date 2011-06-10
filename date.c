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
	time_t t;

	t = time(NULL);
	while((c = getopt(argc, argv, "d:")) != -1)
		switch(c) {
		case 'd':
			t = estrtol(optarg, 0);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind < argc && argv[optind][0] == '+')
		fmt = &argv[optind][1];
	if(!(now = localtime(&t)))
		eprintf("localtime failed\n");

	strftime(buf, sizeof buf, fmt, now);
	puts(buf);
	return EXIT_SUCCESS;
}
