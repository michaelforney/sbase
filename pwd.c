/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static const char *getpwd(const char *cwd);

int
main(int argc, char *argv[])
{
	char *cwd, c;
	char mode = 'L';

	while((c = getopt(argc, argv, "LP")) != -1)
		switch(c) {
		case 'L':
		case 'P':
			mode = c;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	cwd = agetcwd();
	puts((mode == 'L') ? getpwd(cwd) : cwd);
	return EXIT_SUCCESS;
}

const char *
getpwd(const char *cwd)
{
	const char *pwd;
	struct stat cst, pst;

	if(!(pwd = getenv("PWD")) || pwd[0] != '/' || stat(pwd, &pst) == -1)
		return cwd;
	if(stat(cwd, &cst) == -1)
		eprintf("stat %s:", cwd);
	if(pst.st_dev == cst.st_dev && pst.st_ino == cst.st_ino)
		return pwd;
	else
		return cwd;
}
