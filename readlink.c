/* See LICENSE file for copyright and license details. */
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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
			strncat(arg, argv[0], PATH_MAX);
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
					strncat(b, lp, PATH_MAX);
				}
			}
			break;
		case 'e':
			if (stat(argv[0], &st) < 0)
				exit(1);
		case 'f':
			if (!realpath(argv[0], b))
				exit(1);
			break;
		default:
			if ((n = readlink(argv[0], b, PATH_MAX - 1)) < 0)
				exit(1);
			b[n] = '\0';
	}
	printf("%s", b);
	if (!nflag)
		putchar('\n');

	return 0;
}
