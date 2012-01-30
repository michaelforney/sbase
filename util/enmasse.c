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
		fnck(argv[0], argv[1], fn);
		return;
	}
	else
		dir = (argc == 1) ? "." : argv[--argc];

	apathmax(&buf, &size);
	for(i = 0; i < argc; i++) {
		if(snprintf(buf, size, "%s/%s", dir, basename(argv[i])) > size)
			eprintf("%s/%s: filename too long\n", dir, basename(argv[i]));
		fnck(argv[i], buf, fn);
	}
	free(buf);
}
