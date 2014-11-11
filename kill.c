/* See LICENSE file for copyright and license details. */
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
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

const char *sig2name(int);

static void
usage(void)
{
	weprintf("usage: %s [-s signame | -signum | -signame] pid ...\n", argv0);
	weprintf("       %s -l [exit_status]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *end;
	const char *name;
	int ret = 0;
	int sig = SIGTERM;
	pid_t pid;
	size_t i;

	argv0 = argv[0];
	if (argc < 2)
		usage();

	argc--;
	argv++;
	if (strcmp(argv[0], "--") == 0) {
		argc--;
		argv++;
	} else if (argv[0][0] == '-' && isdigit(argv[0][1])) {
		/* handle XSI extension -signum */
		errno = 0;
		sig = strtol(&argv[0][1], &end, 0);
		if (*end != '\0' || errno != 0)
			eprintf("%d: bad signal number\n", sig);
		for (i = 0; i < LEN(sigs); i++)
			if (sigs[i].sig == sig || sig == 0)
				break;
		if (i == LEN(sigs))
			eprintf("%d: bad signal number\n", sig);
		argc--;
		argv++;
	} else if (strcmp(argv[0], "-l") == 0) {
		argc--;
		argv++;
		if (argc == 0) {
			for (i = 0; i < LEN(sigs); i++)
				puts(sigs[i].name);
			exit(0);
		} else if (argc > 1)
			usage();
		errno = 0;
		sig = strtol(argv[0], &end, 0);
		if (*end == '\0' && errno == 0) {
			name = sig2name(sig);
			if (!name)
				eprintf("%d: bad signal number\n", sig);
			else
				puts(name);
		} else {
			eprintf("%s: bad signal name\n", argv[0]);
		}
		exit(0);
	} else {
		if (strcmp(argv[0], "-s") == 0) {
			argc--;
			argv++;
			if (argc == 0)
				usage();
			name = argv[0];
		} else {
			/* assume XSI extension -signame */
			name = &argv[0][1];
		}
		if (strcmp(name, "0") == 0) {
			sig = 0;
		} else {
			for (i = 0; i < LEN(sigs); i++) {
				if (strcasecmp(sigs[i].name, name) == 0) {
					sig = sigs[i].sig;
					break;
				}
			}
			if (i == LEN(sigs))
				eprintf("%s: bad signal name\n", name);
		}
		argc--;
		argv++;
	}

	if (argc == 0)
		usage();

	for (; argc; argc--, argv++) {
		errno = 0;
		pid = strtol(argv[0], &end, 0);
		if (*end == '\0' && errno == 0) {
			if (kill(pid, sig) < 0) {
				weprintf("kill %d:", pid);
				ret = 1;
			}
		} else {
			weprintf("%s: bad pid\n", argv[0]);
			ret = 1;
		}
	}

	exit(ret);
}

const char *
sig2name(int sig)
{
	size_t i;

	if (sig > 128)
		sig = WTERMSIG(sig);
	for (i = 0; i < LEN(sigs); i++)
		if (sigs[i].sig == sig)
			return sigs[i].name;
	return NULL;
}
