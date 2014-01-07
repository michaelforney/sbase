/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

enum {
	NARGS = 5000
};

static int inputc(void);
static void deinputc(int);
static void fillargbuf(int);
static int eatspace(void);
static int parsequote(int);
static int parseescape(void);
static char *poparg(void);
static void pusharg(char *);
static void waitchld(void);
static void runcmd(void);

static char **cmd;
static char *argb;
static size_t argbsz = 1;
static size_t argbpos;
static int nerrors = 0;
static char *eofstr;
static int rflag = 0;

static void
usage(void)
{
	eprintf("usage: %s [-r] [-E eofstr] [cmd [arg...]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	long argsz, argmaxsz;
	char *arg;
	int i;

	ARGBEGIN {
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

	cmd = malloc(NARGS * sizeof(*cmd));
	if (!cmd)
		eprintf("malloc:");

	argb = malloc(argbsz);
	if (!argb)
		eprintf("malloc:");

	do {
		argsz = 0; i = 0;
		if (argc > 0) {
			for (; i < argc; i++) {
				cmd[i] = strdup(argv[i]);
				argsz += strlen(cmd[i]) + 1;
			}
		} else {
			cmd[i] = strdup("/bin/echo");
			argsz += strlen(cmd[i]) + 1;
			i++;
		}
		while ((arg = poparg())) {
			if (argsz + strlen(arg) + 1 > argmaxsz ||
			    i >= NARGS - 1) {
				pusharg(arg);
				break;
			}
			cmd[i] = strdup(arg);
			argsz += strlen(cmd[i]) + 1;
			i++;
		}
		cmd[i] = NULL;
		if (i == 1 && rflag == 1); else runcmd();
		for (; i >= 0; i--)
			free(cmd[i]);
	} while (arg);

	free(argb);
	free(cmd);

	return nerrors > 0 ? 123 : 0;
}

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
		argbsz *= 2;
		argb = realloc(argb, argbsz);
		if (!argb)
			eprintf("realloc:");
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
		if (ch == q) {
			fillargbuf('\0');
			return 0;
		}
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
	if (eatspace() == -1)
		return NULL;
	while ((ch = inputc()) != EOF) {
		switch (ch) {
		case ' ': case '\t': case '\n':
			deinputc(ch);
			goto out;
		case '\'':
			if (parsequote('\'') == -1)
				enprintf(EXIT_FAILURE,
					 "unterminated single quote\n");
			break;
		case '\"':
			if (parsequote('\"') == -1)
				enprintf(EXIT_FAILURE,
					 "unterminated double quote\n");
			break;
		case '\\':
			if (parseescape() == -1)
				enprintf(EXIT_FAILURE, "backslash at EOF\n");
			break;
		default:
			fillargbuf(ch);
			argbpos++;
			break;
		}
	}
out:
	if (argbpos > 0) {
		fillargbuf('\0');
		if (eofstr && strcmp(argb, eofstr) == 0)
			return NULL;
		return argb;
	}
	return NULL;
}

static void
pusharg(char *arg)
{
	char *p;

	for (p = &arg[strlen(arg) - 1]; p >= arg; p--)
		deinputc(*p);
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
runcmd(void)
{
	pid_t pid;
	int saved_errno;

	pid = fork();
	if (pid < 0)
		eprintf("fork:");
	if (pid == 0) {
		execvp(*cmd, cmd);
		saved_errno = errno;
		weprintf("execvp %s:", *cmd);
		_exit(saved_errno == ENOENT ? 127 : 126);
	}
	waitchld();
}
