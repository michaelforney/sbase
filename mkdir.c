/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static void mkdirp(char *);

int
main(int argc, char *argv[])
{
	bool pflag = false;
	char c;

	while((c = getopt(argc, argv, "p")) != -1)
		switch(c) {
		case 'p':
			pflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	for(; optind < argc; optind++)
		if(pflag)
			mkdirp(argv[optind]);
		else if(mkdir(argv[optind], S_IRWXU|S_IRWXG|S_IRWXO) == -1)
			eprintf("mkdir %s:", argv[optind]);
	return EXIT_SUCCESS;
}

void
mkdirp(char *path)
{
	char *p = path;

	do {
		if((p = strchr(&p[1], '/')))
			*p = '\0';
		if(mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO) == -1 && errno != EEXIST)
			eprintf("mkdir %s:", path);
		if(p)
			*p = '/';
	} while(p);
}
