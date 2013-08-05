#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <utmp.h>
#include "util.h"

static void usage(void);

int
main(int argc, char **argv)
{
	struct utmp usr;
	FILE *ufp;
	time_t t;
	char timebuf[sizeof "yyyy-mm-dd hh:mm"];
	bool mflag = false;

	ARGBEGIN {
	case 'm':
		mflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 0)
		usage();

	if (!(ufp = fopen(_PATH_UTMP, "r"))) {
		eprintf("fopen:");
	}
	while(fread((char *)&usr, sizeof(usr), 1, ufp) == 1) {
		if (!*usr.ut_name || !*usr.ut_line)
			continue;
		if (mflag && strcmp(usr.ut_line,
				    strrchr(ttyname(STDIN_FILENO), '/') + 1))
			continue;
		t = usr.ut_time;
		strftime(timebuf, sizeof timebuf, "%Y-%m-%d %H:%M", localtime(&t));
		printf("%-8s %-12s %-16s\n", usr.ut_name, usr.ut_line, timebuf);
	}
	fclose(ufp);
	return 0;
}

void
usage(void)
{
	eprintf("usage: who [-m]\n");
}
