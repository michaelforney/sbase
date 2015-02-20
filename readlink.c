/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-efmn] name\n", argv0);
}

int
main(int argc, char *argv[])
{
	char buf1[PATH_MAX], buf2[PATH_MAX], arg[PATH_MAX];
	int nflag = 0;
	int mefflag = 0;
	ssize_t n;
	struct stat st;
	char *p = arg, *lp = NULL, *b = buf1;

	ARGBEGIN {
	case 'm':
	case 'e':
	case 'f':
		mefflag = ARGC();
		break;
	case 'n':
		nflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 1)
		usage();

	if (strlen(argv[0]) > PATH_MAX - 1)
		eprintf("path too long\n");

#define SWAP_BUF() (b = (b == buf1 ? buf2 : buf1));
	switch (mefflag) {
	case 'm':
		if (argv[0][0] == '/') { /* case when path is on '/' */
			arg[0] = '/';
			arg[1] = '\0';
			p++;
		} else if (!strchr(argv[0], '/')) { /* relative path */
			arg[0] = '.';
			arg[1] = '/';
			arg[2] = '\0';
		} else
			arg[0] = '\0';
		if (strlcat(arg, argv[0], PATH_MAX) >= PATH_MAX)
			eprintf("path too long\n");
		while ((p = strchr(p, '/'))) {
			*p = '\0';
			if (!realpath(arg, b)) {
				*p = '/';
				goto mdone;
			}
			SWAP_BUF();
			lp = p;
			*p++ = '/';
		}
		if (!realpath(arg, b)) {
mdone:
			SWAP_BUF();
			if (lp) {
				/* drop the extra '/' on root */
				lp += (argv[0][0] == '/' &&
				       lp - arg == 1);
				if (strlcat(b, lp, PATH_MAX) >= PATH_MAX)
					eprintf("path too long\n");
			}
		}
		break;
	case 'e':
		if (stat(argv[0], &st) < 0)
			eprintf("stat %s:", argv[0]);
	case 'f':
		if (!realpath(argv[0], b))
			eprintf("realpath %s:", argv[0]);
		break;
	default:
		if ((n = readlink(argv[0], b, PATH_MAX - 1)) < 0)
			eprintf("readlink %s:", argv[0]);
		b[n] = '\0';
	}
	printf("%s", b);
	if (!nflag)
		putchar('\n');

	return 0;
}
