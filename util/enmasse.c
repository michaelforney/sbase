/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../util.h"

static void fnck(const char *, const char *, int (*)(const char *, const char *));

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
	else if(argc == 1)
		dir = ".";
	else
		dir = argv[--argc];

	if((size = pathconf(dir, _PC_PATH_MAX)) == -1)
		size = BUFSIZ;
	if(!(buf = malloc(size)))
		eprintf("malloc:");
	for(i = 0; i < argc; i++) {
		snprintf(buf, size, "%s/%s", dir, basename(argv[i]));
		fnck(argv[i], buf, fn);
	}
	free(buf);
}

void
fnck(const char *a, const char *b, int (*fn)(const char *, const char *))
{
	struct stat sta, stb;

	if(stat(a, &sta) == 0 && stat(b, &stb) == 0
	&& sta.st_dev == stb.st_dev && sta.st_ino == stb.st_ino)
		eprintf("%s: same file as: %s\n", b, a);
	if(fn(a, b) == -1)
		eprintf("%s:", b);
}
