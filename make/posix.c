#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "make.h"


static volatile pid_t pid;

void
killchild(void)
{
	if (pid != 0)
		kill(pid, SIGTERM);
	pid = 0;
}

int
is_dir(char *fname)
{
	struct stat st;

	if (stat(fname, &st) < 0)
		return 0;
	return S_ISDIR(st.st_mode);
}

void
exportvar(char *var, char *value)
{
	int n;
	char *buf;

	n = snprintf(NULL, 0, "%s=%s", var, value);
	buf = emalloc(n+1);
	snprintf(buf, n+1, "%s=%s", var, value);
	putenv(buf);
}

time_t
stamp(char *name)
{
	struct stat st;

	if (stat(name, &st) < 0)
		return -1;

	return st.st_mtime;
}

int
launch(char *cmd, int ignore)
{
	int st;
	sigset_t new, old;
	char *name, *shell;
	char *args[] = {NULL, "-ec" , cmd, NULL};
	static int initsignals;
	extern char **environ;
	extern void sighandler(int);

	if (!initsignals) {
		struct sigaction act = {
			.sa_handler = sighandler
		};

		/* avoid BSD weirdness signal restart handling */
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGHUP, &act, NULL);
		sigaction(SIGTERM, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		initsignals = 1;
	}

	sigfillset(&new);
	sigprocmask(SIG_BLOCK, &new, &old);
	if (stop)
		goto unblock;

	switch (pid = fork()) {
	case -1:
		perror("make");
	unblock:
		sigprocmask(SIG_SETMASK, &old, NULL);
		return -1;
	case 0:
		signal(SIGINT, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);

		sigprocmask(SIG_SETMASK, &old, NULL);

		shell = getmacro("SHELL");

		if (ignore)
			args[1] = "-c";
		if ((name = strrchr(shell, '/')) != NULL)
			++name;
		else
			name = shell;
		args[0] = name;
		execve(shell, args, environ);
		_exit(127);
	default:
		sigprocmask(SIG_SETMASK, &old, NULL);
		wait(&st);

		return st;
	}
}
