/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static void rm(const char *);

static bool rflag = 0;

int
main(int argc, char *argv[])
{
	char c;

	while((c = getopt(argc, argv, "fr")) != -1)
		switch(c) {
		case 'f':
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
		struct dirent *d;
		DIR *dp;

		if(!(dp = opendir(path)))
			eprintf("opendir %s:", path);
		if(chdir(path) != 0)
			eprintf("chdir %s:", path);
		while((d = readdir(dp)))
			if(strcmp(d->d_name, ".") && strcmp(d->d_name, ".."))
				rm(d->d_name);

		closedir(dp);
		if(chdir("..") != 0)
			eprintf("chdir:");
		if(remove(path) == 0)
			return;
	}
	eprintf("remove %s:", path);
}
