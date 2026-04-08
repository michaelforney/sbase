/* See LICENSE file for copyright and license details. */
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

#define NARGS 10000

static int inputc(void);
static void fillargbuf(int);
static int eatspace(void);
static int parsequote(int);
static int parseescape(void);
static char *poparg(void);
static void waitchld(int);
static void spawn(void);

static size_t argbsz;
static size_t argbpos;
static size_t maxargs;
static size_t curprocs, maxprocs = 1;
static int    nerrors;
static int    nulflag, nflag, pflag, rflag, tflag, xflag, Iflag;
static char  *argb;
static char  **cmd;
static char  *eofstr;

static int
inputc(void)
{
	int ch;

	ch = getc(stdin);
	if (ch == EOF && ferror(stdin))
		eprintf("getc <stdin>:");

	return ch;
}

static void
fillargbuf(int ch)
{
	if (argbpos >= argbsz) {
		argbsz = argbpos == 0 ? 1 : argbsz * 2;
		argb = erealloc(argb, argbsz);
	}
	argb[argbpos] = ch;
}

static int
eatspace(void)
{
	int ch;

	while ((ch = inputc()) != EOF) {
		if (nulflag || !(ch == ' ' || ch == '\t' || ch == '\n')) {
			ungetc(ch, stdin);
			return ch;
		}
	}
	return -1;
}

static int
parsequote(int q)
{
	int ch;

	while ((ch = inputc()) != EOF) {
		if (ch == q)
			return 0;
		if (ch != '\n') {
			fillargbuf(ch);
			argbpos++;
		}
	}

	return -1;
}

static int
parseescape(void)
{
	int ch;

	if ((ch = inputc()) != EOF) {
		fillargbuf(ch);
		argbpos++;
		return ch;
	}

	return -1;
}

static char *
poparg(void)
{
	int ch;

	argbpos = 0;
	if (eatspace() < 0)
		return NULL;
	while ((ch = inputc()) != EOF) {
		/* NUL separator: no escaping */
		if (nulflag) {
			if (ch == '\0')
				goto out;
			else
				goto fill;
		}

		switch (ch) {
		case ' ':
		case '\t':
			if (Iflag)
				goto fill;
		case '\n':
			goto out;
		case '\'':
			if (parsequote('\'') < 0)
				eprintf("unterminated single quote\n");
			break;
		case '\"':
			if (parsequote('\"') < 0)
				eprintf("unterminated double quote\n");
			break;
		case '\\':
			if (parseescape() < 0)
				eprintf("backslash at EOF\n");
			break;
		default:
		fill:
			fillargbuf(ch);
			argbpos++;
			break;
		}
	}
out:
	fillargbuf('\0');

	return (eofstr && !strcmp(argb, eofstr)) ? NULL : argb;
}

static void
waitchld(int waitall)
{
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, !waitall && curprocs < maxprocs ?
	       WNOHANG : 0)) > 0) {
	       curprocs--;

		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 255)
				exit(124);
			if (WEXITSTATUS(status) == 127 ||
			    WEXITSTATUS(status) == 126)
				exit(WEXITSTATUS(status));
			if (WEXITSTATUS(status))
				nerrors++;
		}
		if (WIFSIGNALED(status))
			exit(125);
	}
	if (pid == -1 && errno != ECHILD)
		eprintf("waitpid:");
}

static int
prompt(void)
{
	FILE *fp;
	int ch, ret;

	if (!(fp = fopen("/dev/tty", "r")))
		return -1;

	fputs("?...", stderr);
	fflush(stderr);

	ch = fgetc(fp);
	ret = (ch == 'y' || ch == 'Y');
	if (ch != EOF && ch != '\n') {
		while ((ch = fgetc(fp)) != EOF) {
			if (ch == '\n')
				break;
		}
	}

	fclose(fp);

	return ret;
}

static void
spawn(void)
{
	int savederrno;
	int first = 1;
	char **p;

	if (pflag || tflag) {
		for (p = cmd; *p; p++) {
			if (!first)
				fputc(' ', stderr);
			fputs(*p, stderr);
			first = 0;
		}
		if (pflag) {
			switch (prompt()) {
			case -1: break; /* error */
			case 0: return; /* no */
			case 1: goto dospawn; /* yes */
			}
		}
		fputc('\n', stderr);
		fflush(stderr);
	}

dospawn:
	switch (fork()) {
	case -1:
		eprintf("fork:");
	case 0:
		execvp(*cmd, cmd);
		savederrno = errno;
		weprintf("execvp %s:", *cmd);
		_exit(126 + (savederrno == ENOENT));
	}
	curprocs++;
	waitchld(0);
}

static void
usage(void)
{
	eprintf("usage: %s [-0prtx] [-E eofstr] [-n num] [-P maxprocs] [-s num] "
	        "[cmd [arg ...]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int ret = 0, leftover = 0, i, j;
	size_t argsz, argmaxsz;
	size_t arglen, a;
	char *arg = "";
	char *replstr;

	if ((argmaxsz = sysconf(_SC_ARG_MAX)) == (size_t)-1)
		argmaxsz = _POSIX_ARG_MAX;
	/* Leave some room for environment variables */
	argmaxsz -= 4096;
	cmd = emalloc(NARGS * sizeof(*cmd));

	ARGBEGIN {
	case '0':
		nulflag = 1;
		break;
	case 'n':
		nflag = 1;
		maxargs = estrtonum(EARGF(usage()), 1, MIN(SIZE_MAX, LLONG_MAX));
		break;
	case 'p':
		pflag = 1;
		break;
	case 'r':
		rflag = 1;
		break;
	case 's':
		argmaxsz = estrtonum(EARGF(usage()), 1, MIN(SIZE_MAX, LLONG_MAX));
		break;
	case 't':
		tflag = 1;
		break;
	case 'x':
		xflag = 1;
		break;
	case 'E':
		eofstr = EARGF(usage());
		break;
	case 'I':
		Iflag = 1;
		xflag = 1;
		nflag = 1;
		maxargs = 1;
		replstr = EARGF(usage());
		break;
	case 'P':
		maxprocs = estrtonum(EARGF(usage()), 1, MIN(SIZE_MAX, LLONG_MAX));
		break;
	default:
		usage();
	} ARGEND

	do {
		argsz = 0; i = 0; a = 0;
		if (argc) {
			for (; i < argc; i++) {
				cmd[i] = estrdup(argv[i]);
				argsz += strlen(cmd[i]) + 1;
			}
		} else {
			cmd[i] = estrdup("/bin/echo");
			argsz += strlen("/bin/echo") + 1;
			i++;
		}
		while (leftover || (arg = poparg())) {
			arglen = strlen(arg);
			if (argsz + arglen >= argmaxsz || i >= NARGS - 1) {
				if (xflag || arglen >= argmaxsz || leftover)
					eprintf("insufficient argument space\n");
				leftover = 1;
				break;
			}

			if (!Iflag) {
				cmd[i] = estrdup(arg);
				argsz += arglen + 1;
			} else {
				for (j = 1; j < i; j++) {
					char *p = cmd[j];
					argsz -= strlen(cmd[j]);
					strnsubst(&cmd[j], replstr, arg, 255);
					argsz += strlen(cmd[j]);
					free(p);
				}
			}

			i++;
			a++;
			leftover = 0;
			if (nflag && a >= maxargs)
				break;
		}
		cmd[i] = NULL;
		if (a >= maxargs && nflag)
			spawn();
		else if (!a || (i == 1 && rflag))
			;
		else
			spawn();
		for (; i >= 0; i--)
			free(cmd[i]);
	} while (arg);

	free(argb);

	waitchld(1);

	if (nerrors || (fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>")))
		ret = 123;

	return ret;
}
