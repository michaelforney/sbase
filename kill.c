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

static void
usage(void)
{
	eprintf("usage: %s [-s signal] [pid...]\n"
	        "       %s -l [signum]\n", argv0, argv0);
}

int
main(int argc, char *argv[])
{
	bool lflag = false;
	char *end, *v;
	int sig = SIGTERM;
	pid_t pid;
	size_t i;

	ARGBEGIN {
	case 'l':
		lflag = true;
		break;
	case 's':
		v = EARGF(usage());
		sig = strtol(v, &end, 0);
		if(*end == '\0')
			break;
		for(i = 0; i < LEN(sigs); i++) {
			if(!strcasecmp(v, sigs[i].name)) {
				sig = sigs[i].sig;
				break;
			}
		}
		if(i == LEN(sigs))
			eprintf("%s: unknown signal\n", v);
		break;
	default:
		usage();
	} ARGEND;

	if(argc < 1)
		usage();

	if(lflag) {
		sig = (argc > 0) ? 0 : estrtol(argv[0], 0);
		if(sig > 128)
			sig = WTERMSIG(sig);
		for(i = 0; i < LEN(sigs); i++)
			if(sigs[i].sig == sig || sig == 0)
				putword(sigs[i].name);
		putchar('\n');
	} else for(; argc > 0; argc--, argv++) {
		pid = estrtol(argv[0], 0);
		if(kill(pid, sig) == -1)
			eprintf("kill %d:", pid);
	}

	return EXIT_SUCCESS;
}
