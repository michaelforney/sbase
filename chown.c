/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static void chownpwgr(const char *);

static int rflag = 0;
static uid_t uid = -1;
static gid_t gid = -1;
static int ret = 0;

static void
usage(void)
{
	eprintf("usage: %s [-Rr] [owner][:[group]] file...\n", argv0);
}

int
main(int argc, char *argv[])
{
	char *owner, *group, *end;
	struct passwd *pw;
	struct group *gr;

	ARGBEGIN {
	case 'R':
	case 'r':
		rflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		usage();

	owner = argv[0];
	argv++;
	argc--;
	if ((group = strchr(owner, ':')))
		*group++ = '\0';

	if (owner && *owner) {
		errno = 0;
		pw = getpwnam(owner);
		if (pw) {
			uid = pw->pw_uid;
		} else {
			if (errno != 0)
				eprintf("getpwnam %s:", owner);
			uid = strtoul(owner, &end, 10);
			if (*end != '\0')
				eprintf("getpwnam %s: no such user\n", owner);
		}
	}
	if (group && *group) {
		errno = 0;
		gr = getgrnam(group);
		if (gr) {
			gid = gr->gr_gid;
		} else {
			if (errno != 0)
				eprintf("getgrnam %s:", group);
			gid = strtoul(group, &end, 10);
			if (*end != '\0')
				eprintf("getgrnam %s: no such group\n", group);
		}
	}
	for (; argc > 0; argc--, argv++)
		chownpwgr(argv[0]);

	return ret;
}

void
chownpwgr(const char *path)
{
	if (chown(path, uid, gid) == -1) {
		weprintf("chown %s:", path);
		ret = 1;
	}
	if (rflag)
		recurse(path, chownpwgr);
}
