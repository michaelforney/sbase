/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	bool mflag = false;
	bool nflag = false;
	bool rflag = false;
	bool sflag = false;
	bool vflag = false;
	char c;
	struct utsname u;

	while((c = getopt(argc, argv, "amnrsv")) != -1)
		switch(c) {
		case 'a':
			mflag = nflag = rflag = sflag = vflag = true;
			break;
		case 'm':
			mflag = true;
			break;
		case 'n':
			nflag = true;
			break;
		case 'r':
			rflag = true;
			break;
		case 's':
			sflag = true;
			break;
		case 'v':
			vflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(uname(&u) == -1)
		eprintf("uname:");

	if(sflag || !(nflag || rflag || vflag || mflag))
		putword(u.sysname);
	if(nflag)
		putword(u.nodename);
	if(rflag)
		putword(u.release);
	if(vflag)
		putword(u.version);
	if(mflag)
		putword(u.machine);
	putchar('\n');
	return EXIT_SUCCESS;
}
