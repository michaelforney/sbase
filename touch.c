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
static struct timespec times[2];

static void
touch(const char *file)
{
	int fd;
	struct stat st;
	int r;

	if ((r = stat(file, &st)) < 0) {
		if (errno != ENOENT)
			eprintf("stat %s:", file);
		if (cflag)
			return;
	} else if (!r) {
		if (!aflag)
			times[0] = st.st_atim;
		if (!mflag)
			times[1] = st.st_mtim;
		if (utimensat(AT_FDCWD, file, times, 0) < 0)
			eprintf("utimensat %s:", file);
		return;
	}

	if ((fd = open(file, O_CREAT | O_EXCL, 0644)) < 0)
		eprintf("open %s:", file);
	close(fd);

	touch(file);
}

static time_t
parsetime(char *str, time_t current)
{
	struct tm *cur, t;
	int zulu = 0;
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
		str[19] = 0;
		zulu = 1;
		format = "%Y-%m-%dT%H:%M:%S";
		break;
	default:
		eprintf("Invalid date format length\n", str);
	}

	if (!strptime(str, format, &t))
		weprintf("strptime %s: Invalid date format\n", str);
	if (zulu) {
		t.tm_hour += t.tm_gmtoff / 60;
		t.tm_gmtoff = 0;
		t.tm_zone = "Z";
	}

	return mktime(&t);
}

static void
usage(void)
{
	eprintf("usage: %s [-acm] [-d time | -r ref_file | -t time | -T time] "
	        "file ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	char *ref = NULL;
	clock_gettime(CLOCK_REALTIME, &times[0]);

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	case 'c':
		cflag = 1;
		break;
	case 'd':
	case 't':
		times[0].tv_sec = parsetime(EARGF(usage()), times[0].tv_sec);
		break;
	case 'm':
		mflag = 1;
		break;
	case 'r':
		ref = EARGF(usage());
		if (stat(ref, &st) < 0)
			eprintf("stat '%s':", ref);
		times[0] = st.st_atim;
		times[1] = st.st_mtim;
		break;
	case 'T':
		times[0].tv_sec = estrtonum(EARGF(usage()), 0, LLONG_MAX);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();
	if (!aflag && !mflag)
		aflag = mflag = 1;
	if (!ref)
		times[1] = times[0];

	for (; *argv; argc--, argv++)
		touch(*argv);

	return 0;
}
