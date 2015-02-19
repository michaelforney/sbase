/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
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

time_t
parsetime(char *str, time_t current)
{
	struct tm *cur, t;
	char *format;
	size_t len = strlen(str);

	cur = localtime(&current);
	t.tm_isdst = -1;

	switch (len) {
	/* -t flag argument */
	case 8:
		t.tm_sec = 0;
		t.tm_year = cur->tm_year;
		format = "%m%d%H%M";
		break;
	case 10:
		t.tm_sec = 0;
		format = "%y%m%d%H%M";
		break;
	case 11:
		t.tm_year = cur->tm_year;
		format = "%m%d%H%M.%S";
		break;
	case 12:
		t.tm_sec = 0;
		format = "%Y%m%d%H%M";
		break;
	case 13:
		format = "%y%m%d%H%M.%S";
		break;
	case 15:
		format = "%Y%m%d%H%M.%S";
		break;
	/* -d flag argument */
	case 19:
		format = "%Y-%m-%dT%H:%M:%S";
		break;
	case 20:
		/* only Zulu-timezone supported */
		if (str[19] != 'Z')
			eprintf("Invalid time zone\n");
		format = "%Y-%m-%dT%H:%M:%S%Z";
		break;
	default:
		eprintf("Invalid date format length\n", str);
	}

	if (!strptime(str, format, &t))
		weprintf("strptime %s: Invalid date format\n", str);

	return mktime(&t);
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
	case 'T':
		t = estrtonum(EARGF(usage()), 0, LLONG_MAX);
		break;
	case 't':
		t = parsetime(EARGF(usage()), t);
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 1)
		usage();
	if (!aflag && !mflag)
		aflag = mflag = 1;

	for (; argc > 0; argc--, argv++)
		touch(argv[0]);

	return 0;
}
