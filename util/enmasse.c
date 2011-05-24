/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../util.h"

void
enmasse(int argc, char **argv, int (*fn)(const char *, const char *))
{
	char *buf, *dir;
	int i;
	long size;
	struct stat st;

	if(argc == 2 && !(stat(argv[1], &st) == 0 && S_ISDIR(st.st_mode))) {
		if(fn(argv[0], argv[1]) != 0)
			eprintf("%s:", argv[1]);
		return;
	}
	else if(argc == 1)
		dir = ".";
	else
		dir = argv[--argc];

	if((size = pathconf(dir, _PC_PATH_MAX)) < 0)
		size = BUFSIZ;
	if(!(buf = malloc(size)))
		eprintf("malloc:");
	for(i = 0; i < argc; i++) {
		snprintf(buf, size, "%s/%s", dir, basename(argv[i]));
		if(fn(argv[i], buf) != 0)
			eprintf("%s:", buf);
	}
	free(buf);
}
