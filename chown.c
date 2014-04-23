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
static int ret = EXIT_SUCCESS;

static void
usage(void)
{
	eprintf("usage: %s [-r] [owner][:[group]] file...\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *owner, *group;

	ARGBEGIN {
	case 'R':
	case 'r':
		rflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if(argc == 0)
		usage();

	owner = argv[0];
	argv++;
	argc--;
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
	for(; argc > 0; argc--, argv++)
		chownpwgr(argv[0]);

	return ret;
}

void
chownpwgr(const char *path)
{
	if(chown(path, pw ? pw->pw_uid : (uid_t)-1,
	               gr ? gr->gr_gid : (gid_t)-1) == -1) {
		weprintf("chown %s:", path);
		ret = EXIT_FAILURE;
	}
	if(rflag)
		recurse(path, chownpwgr);
}
