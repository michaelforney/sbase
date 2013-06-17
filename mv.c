/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fs.h"
#include "util.h"

int mv(const char *, const char *);

static void
usage(void)
{
	eprintf("usage: %s source... dest\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc > 3 && !(stat(argv[argc-1], &st) == 0 && S_ISDIR(st.st_mode)))
		eprintf("%s: not a directory\n", argv[argc-1]);
	enmasse(argc, &argv[0], mv);

	return 0;
}

int
mv(const char *s1, const char *s2)
{
	if (rename(s1, s2) == 0)
		return 0;
	if (errno == EXDEV) {
		cp_rflag = true;
		rm_rflag = true;
		cp(s1, s2);
		rm(s1);
		return 0;
	}
	return -1;
}

