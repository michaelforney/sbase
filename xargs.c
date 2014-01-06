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
static void fillbuf(int);
static void eatspace(void);
static int parsequote(int);
static void parseescape(void);
static char *poparg(void);
static void pusharg(char *);
static void runcmd(void);

static char **cmd;
static char *argb;
static size_t argbsz = 1;
static size_t argbpos;
static int rflag = 0;

static void
usage(void)
{
	eprintf("usage: %s [-r] [cmd [arg...]]\n", argv0);
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
	return 0;
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
fillbuf(int ch)
{
	if (argbpos >= argbsz) {
		argbsz *= 2;
		argb = realloc(argb, argbsz);
		if (!argb)
			eprintf("realloc:");
	}
	argb[argbpos] = ch;
}

static void
eatspace(void)
{
	int ch;

	while ((ch = inputc()) != EOF) {
		switch (ch) {
		case ' ': case '\t': case '\n':
			break;
		default:
			deinputc(ch);
			return;
		}
	}
}

static int
parsequote(int q)
{
	int ch;

	while ((ch = inputc()) != EOF) {
		if (ch == q) {
			fillbuf('\0');
			return 0;
		}
		if (ch != '\n') {
			fillbuf(ch);
			argbpos++;
		}
	}
	return -1;
}

static void
parseescape(void)
{
	int ch;

	if ((ch = inputc()) != EOF) {
		fillbuf(ch);
		argbpos++;
	}
}

static char *
poparg(void)
{
	int ch;

	argbpos = 0;
	eatspace();
	while ((ch = inputc()) != EOF) {
		switch (ch) {
		case ' ': case '\t': case '\n':
			fillbuf('\0');
			deinputc(ch);
			return argb;
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
			parseescape();
			break;
		default:
			fillbuf(ch);
			argbpos++;
			break;
		}
	}
	if (argbpos > 0) {
		fillbuf('\0');
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
runcmd(void)
{
	pid_t pid;
	int status, saved_errno;

	pid = fork();
	if (pid < 0)
		eprintf("fork:");
	if (pid == 0) {
		execvp(*cmd, cmd);
		saved_errno = errno;
		weprintf("execvp %s:", *cmd);
		_exit(saved_errno == ENOENT ? 127 : 126);
	}
	wait(&status);
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 255)
			exit(124);
		if (WEXITSTATUS(status) == 127 ||
		    WEXITSTATUS(status) == 126)
			exit(WEXITSTATUS(status));
	}
	if (WIFSIGNALED(status))
		exit(125);
}
