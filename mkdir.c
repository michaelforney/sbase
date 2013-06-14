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

static void
usage(void)
{
	eprintf("usage: %s [-p] directory...\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	bool pflag = false;

	ARGBEGIN {
	case 'p':
		pflag = true;
		break;
	default:
		usage();
	} ARGEND;

	for(; argc > 0; argc--, argv++) {
		if(pflag) {
			mkdirp(argv[0]);
		} else if(mkdir(argv[0], S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
			eprintf("mkdir %s:", argv[0]);
		}
	}

	return 0;
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

