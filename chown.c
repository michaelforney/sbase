/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static void chownpwgr(const char *);

static bool rflag = false;
static struct passwd *pw = NULL;
static struct group *gr = NULL;

int
main(int argc, char *argv[])
{
	char c, *owner, *group;

	while((c = getopt(argc, argv, "r")) != -1)
		switch(c) {
		case 'r':
			rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc)
		eprintf("usage: %s [-r] [owner][:group] [file...]\n", argv[0]);
	owner = argv[optind++];
	if((group = strchr(owner, ':')))
		*group++ = '\0';

	if(owner && *owner) {
		errno = 0;
		pw = getpwnam(owner);
		if(errno != 0)
			eprintf("getpwnam %s:", owner);
		else if(!pw)
			eprintf("getpwnam %s: no such user\n", owner);
	}
	if(group && *group) {
		errno = 0;
		gr = getgrnam(group);
		if(errno != 0)
			eprintf("getgrnam %s:", group);
		else if(!gr)
			eprintf("getgrnam %s: no such group\n", group);
	}
	for(; optind < argc; optind++)
		chownpwgr(argv[optind]);
	return EXIT_SUCCESS;
}

void
chownpwgr(const char *path)
{
	if(chown(path, pw ? pw->pw_uid : (uid_t)-1,
	               gr ? gr->gr_gid : (gid_t)-1) == -1)
		eprintf("chown %s:", path);
	if(rflag)
		recurse(path, chownpwgr);
}
