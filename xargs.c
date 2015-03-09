/* See LICENSE file for copyright and license details. */
#include <sys/wait.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

enum {
	NARGS = 10000
};

static int inputc(void);
static void deinputc(int);
static void fillargbuf(int);
static int eatspace(void);
static int parsequote(int);
static int parseescape(void);
static char *poparg(void);
static void waitchld(void);
static void spawn(void);

static char *cmd[NARGS];
static char *argb;
static size_t argbsz;
static size_t argbpos;
static long maxargs = 0;
static int nerrors = 0;
static char *eofstr;
static int rflag = 0, nflag = 0;

static int
inputc(void)
{
	int ch;

	ch = getc(stdin);
	if (ch == EOF && ferror(stdin))
		eprintf("stdin: read error:");
	return ch;
}

static void
deinputc(int ch)
{
	ungetc(ch, stdin);
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
		switch (ch) {
		case ' ': case '\t': case '\n':
			break;
		default:
			deinputc(ch);
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
		switch (ch) {
		case ' ': case '\t': case '\n':
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
			fillargbuf(ch);
			argbpos++;
			break;
		}
	}
out:
	fillargbuf('\0');
	if (eofstr && strcmp(argb, eofstr) == 0)
		return NULL;
	return argb;
}

static void
waitchld(void)
{
	int status;

	wait(&status);
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 255)
			exit(124);
		if (WEXITSTATUS(status) == 127 ||
		    WEXITSTATUS(status) == 126)
			exit(WEXITSTATUS(status));
		if (status != 0)
			nerrors++;
	}
	if (WIFSIGNALED(status))
		exit(125);
}

static void
spawn(void)
{
	pid_t pid;
	int savederrno;

	pid = fork();
	if (pid < 0) {
		weprintf("fork:");
		_exit(1);
	}
	if (pid == 0) {
		execvp(*cmd, cmd);
		savederrno = errno;
		weprintf("execvp %s:", *cmd);
		_exit(126 + (savederrno == ENOENT));
	}
	waitchld();
}

static void
usage(void)
{
	eprintf("usage: %s [-n maxargs] [-r] [-E eofstr] [cmd [arg...]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int leftover = 0;
	long argsz, argmaxsz;
	char *arg = "";
	int i, a;

	ARGBEGIN {
	case 'n':
		nflag = 1;
		if ((maxargs = strtol(EARGF(usage()), NULL, 10)) <= 0)
			eprintf("%s: value for -n option should be >= 1\n", argv0);
		break;
	case 'r':
		rflag = 1;
		break;
	case 'E':
		eofstr = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	argmaxsz = sysconf(_SC_ARG_MAX);
	if (argmaxsz < 0)
		eprintf("sysconf:");
	/* Leave some room for environment variables */
	argmaxsz -= 4 * 1024;

	do {
		argsz = 0; i = 0; a = 0;
		if (argc > 0) {
			for (; i < argc; i++) {
				cmd[i] = estrdup(argv[i]);
				argsz += strlen(cmd[i]) + 1;
			}
		} else {
			cmd[i] = estrdup("/bin/echo");
			argsz += strlen(cmd[i]) + 1;
			i++;
		}
		while (leftover == 1 || (arg = poparg())) {
			if (argsz + strlen(arg) + 1 > argmaxsz ||
			    i >= NARGS - 1) {
				if (strlen(arg) + 1 > argmaxsz)
					eprintf("insufficient argument space\n");
				leftover = 1;
				break;
			}
			cmd[i] = estrdup(arg);
			argsz += strlen(cmd[i]) + 1;
			i++;
			a++;
			leftover = 0;
			if (nflag == 1 && a >= maxargs)
				break;
		}
		cmd[i] = NULL;
		if (a >= maxargs && nflag == 1)
			spawn();
		else if (!a || (i == 1 && rflag == 1))
			;
		else
			spawn();
		for (; i >= 0; i--)
			free(cmd[i]);
	} while (arg);

	free(argb);

	return nerrors > 0 ? 123 : 0;
}
