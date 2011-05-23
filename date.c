/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	char *fmt = "%c";
	int i;
	struct tm *now = NULL;
	time_t t;

	t = time(NULL);
	for(i = 1; i < argc; i++)
		if(!strncmp("+", argv[i], 1))
			fmt = &argv[i][1];
		else if(!strcmp("-d", argv[i]) && i+1 < argc)
			t = strtol(argv[++i], NULL, 0);
		else
			eprintf("usage: %s [-d time] [+format]\n", argv[0]);
	now = localtime(&t);
	if(!now)
		eprintf("localtime failed\n");

	strftime(buf, sizeof buf, fmt, now);
	puts(buf);
	return EXIT_SUCCESS;
}
