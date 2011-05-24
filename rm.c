/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include "util.h"

static void rm(const char *);

static bool fflag = false;
static bool rflag = false;

int
main(int argc, char *argv[])
{
	char c;

	while((c = getopt(argc, argv, "fr")) != -1)
		switch(c) {
		case 'f':
			fflag = true;
			break;
		case 'r':
			rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	for(; optind < argc; optind++)
		rm(argv[optind]);
	return EXIT_SUCCESS;
}

void rm(const char *path)
{
	if(remove(path) == 0)
		return;
	if(errno == ENOTEMPTY && rflag) {
		char *buf;
		long size;
		struct dirent *d;
		DIR *dp;

		if((size = pathconf(".", _PC_PATH_MAX)) < 0)
			size = BUFSIZ;
		if(!(buf = malloc(size)))
			eprintf("malloc:");
		if(!getcwd(buf, size))
			eprintf("getcwd:");
		if(!(dp = opendir(path)))
			eprintf("opendir %s:", path);
		if(chdir(path) != 0)
			eprintf("chdir %s:", path);
		while((d = readdir(dp)))
			if(strcmp(d->d_name, ".") && strcmp(d->d_name, ".."))
				rm(d->d_name);

		closedir(dp);
		if(chdir(buf) != 0)
			eprintf("chdir %s:", buf);
		if(remove(path) == 0)
			return;
	}
	if(!fflag)
		eprintf("remove %s:", path);
}
