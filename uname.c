/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "util.h"

static void print(const char *);

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
		print(u.sysname);
	if(nflag)
		print(u.nodename);
	if(rflag)
		print(u.release);
	if(vflag)
		print(u.version);
	if(mflag)
		print(u.machine);
	putchar('\n');
	return EXIT_SUCCESS;
}

void
print(const char *s)
{
	static bool first = true;

	if(!first)
		putchar(' ');
	fputs(s, stdout);
	first = false;
}
