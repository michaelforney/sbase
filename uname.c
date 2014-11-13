/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s [-amnrsv]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int mflag = 0;
	int nflag = 0;
	int rflag = 0;
	int sflag = 0;
	int vflag = 0;
	struct utsname u;

	ARGBEGIN {
	case 'a':
		mflag = nflag = rflag = sflag = vflag = 1;
		break;
	case 'm':
		mflag = 1;
		break;
	case 'n':
		nflag = 1;
		break;
	case 'r':
		rflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'v':
		vflag = 1;
		break;
	default:
		usage();
	} ARGEND;
	if (uname(&u) < 0)
		eprintf("uname:");

	if (sflag || !(nflag || rflag || vflag || mflag))
		putword(u.sysname);
	if (nflag)
		putword(u.nodename);
	if (rflag)
		putword(u.release);
	if (vflag)
		putword(u.version);
	if (mflag)
		putword(u.machine);
	putchar('\n');

	return 0;
}
