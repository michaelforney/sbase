#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <utmpx.h>
#include "util.h"

static void usage(void);

int
main(int argc, char **argv)
{
	struct utmpx *ut;
	time_t t;
	char timebuf[sizeof "yyyy-mm-dd hh:mm"];

	if(argc!=1)
		usage();

	while((ut=getutxent())) {
		if(ut->ut_type != USER_PROCESS)
			continue;
		t = ut->ut_tv.tv_sec;
		strftime(timebuf, sizeof timebuf, "%Y-%m-%d %H:%M", localtime(&t));
		printf("%-8s %-12s %-16s\n", ut->ut_user, ut->ut_line, timebuf);
	}
	endutxent();
	return 0;
}

void 
usage(void)
{
	eprintf("usage: who\n");
}

