/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include "util.h"

static int aflag;
static int cflag;
static int mflag;
static time_t t;

static void
touch(const char *file)
{
	int fd;
	struct stat st;
	struct utimbuf ut;
	int r;

	if ((r = stat(file, &st)) < 0) {
		if (errno != ENOENT)
			eprintf("stat %s:", file);
		if (cflag)
			return;
	} else if (r == 0) {
		ut.actime = aflag ? t : st.st_atime;
		ut.modtime = mflag ? t : st.st_mtime;
		if (utime(file, &ut) < 0)
			eprintf("utime %s:", file);
		return;
	}

	if ((fd = open(file, O_CREAT | O_EXCL, 0644)) < 0)
		eprintf("open %s:", file);
	close(fd);

	touch(file);
}

static void
usage(void)
{
	eprintf("usage: %s [-acm] [-r ref_file | -t timestamp] file ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	char *ref;
	t = time(NULL);

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 'c':
		cflag = 1;
		break;
	case 'm':
		mflag = 1;
		break;
	case 'r':
		ref = EARGF(usage());
		if (stat(ref, &st) < 0)
			eprintf("stat '%s':", ref);
		t = st.st_mtime;
		break;
	case 't':
		t = estrtonum(EARGF(usage()), 0, LLONG_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();

	for (; argc > 0; argc--, argv++)
		touch(argv[0]);

	return 0;
}
