/* See LICENSE file for copyright and license details. */
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>
#include "util.h"

struct {
	const char *name;
	int sig;
} sigs[] = {
#define SIG(n) { #n, SIG##n }
	SIG(ABRT), SIG(ALRM), SIG(BUS),  SIG(CHLD), SIG(CONT), SIG(FPE),  SIG(HUP),
	SIG(ILL),  SIG(INT),  SIG(KILL), SIG(PIPE), SIG(QUIT), SIG(SEGV), SIG(STOP),
	SIG(TERM), SIG(TSTP), SIG(TTIN), SIG(TTOU), SIG(USR1), SIG(USR2), SIG(URG),
#undef SIG
};

static void usage(void);

int
main(int argc, char *argv[])
{
	bool lflag = false;
	char c, *end;
	int sig = SIGTERM;
	pid_t pid;
	size_t i;

	while((c = getopt(argc, argv, "ls:")) != -1)
		switch(c) {
		case 'l':
			lflag = true;
			break;
		case 's':
			sig = strtol(optarg, &end, 0);
			if(*end == '\0')
				break;
			for(i = 0; i < LEN(sigs); i++)
				if(!strcasecmp(optarg, sigs[i].name)) {
					sig = sigs[i].sig;
					break;
				}
			if(i == LEN(sigs))
				eprintf("%s: unknown signal\n", optarg);
			break;
		default:
			usage();
		}
	if(optind < argc-1)
		usage();

	if(lflag) {
		sig = (optind == argc) ? 0 : estrtol(argv[optind], 0);
		if(sig > 128)
			sig = WTERMSIG(sig);
		for(i = 0; i < LEN(sigs); i++)
			if(sigs[i].sig == sig || sig == 0)
				putword(sigs[i].name);
		putchar('\n');
	}
	else for(; optind < argc; optind++) {
		pid = estrtol(argv[optind], 0);
		if(kill(pid, sig) == -1)
			eprintf("kill %d:", pid);
	}
	return EXIT_SUCCESS;
}

void
usage(void)
{
	eprintf("usage: %s [-s signal] [pid...]\n"
	        "       %s -l [signum]\n", argv0, argv0);
}
