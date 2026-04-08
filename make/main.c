#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "make.h"

#ifndef SIGINT
#define SIGINT -1
#endif

#ifndef SIGTERM
#define SIGTERM -1
#endif

#ifndef SIGQUIT
#define SIGQUIT -1
#endif

#ifndef SIGHUP
#define SIGHUP -1
#endif

int kflag, dflag, nflag, iflag, sflag;
int eflag, pflag, tflag, qflag;
int exitstatus;
volatile sig_atomic_t  stop;

void
debug(char *fmt, ...)
{
	va_list va;

	if (!dflag)
		return;

	va_start(va, fmt);
	vfprintf(stdout, fmt, va);
	fputc('\n', stdout);
	va_end(va);
}

int
hash(char *name)
{
	int c;
	unsigned h = 5381;

	while (c = *name++)
		h = h*33 ^ c;

	return h;
}

void *
emalloc(size_t siz)
{
	void *p;

	if ((p = malloc(siz)) == NULL) {
		perror("make");
		exit(EXIT_FAILURE);
	}

	return p;
}

void *
erealloc(void *p, size_t siz)
{
	if ((p = realloc(p, siz)) == NULL) {
		perror("make");
		exit(EXIT_FAILURE);
	}

	return p;
}

char *
estrdup(char *s)
{
	size_t len;

	len = strlen(s) + 1;
	return memcpy(emalloc(len), s, len);
}

void
sighandler(int signo)
{
	killchild();
	stop = signo;
}

static void
usage(void)
{
	fputs("usage: make [-eiknprSstd] [-f file] [-j jobs] "
	      "[macro=value ...] [target ...]\n",
	      stderr);
	exit(EXIT_FAILURE);
}

static char *
getarg(char **args, char ***argv)
{
	char *s;

	if ((*args)[1]) {
		s = (*args) + 1;
		*args += strlen(*args) - 1;
		return s;
	}

	if (!argv)
		usage();

	if ((*argv)[1] == NULL)
		usage();
	(*argv)++;

	return **argv;
}

static void
appendmakeflags(char *text)
{
	int n;
	char *s, *t, *fmt;

	s = getmacro("MAKEFLAGS");
	fmt = *s ? "%s %s" : "%s%s";
	n = snprintf(NULL, 0, fmt, s, text);

	t = emalloc(n+1);
	snprintf(t, n+1, fmt, s, text);
	setmacro("MAKEFLAGS", t, MAKEFLAGS, EXPORT);

	free(t);
}

static int
hasargs(int c)
{
	return c == 'f' || c == 'j';
}

static void
parseflag(int flag, char **args, char ***argv)
{
	if (hasargs(flag))
		getarg(args, argv);

	switch (flag) {
	case 'j':
	case 'f':
		break;
	case 'e':
		eflag = 1;
		appendmakeflags("-e");
		break;
	case 'i':
		iflag = 1;
		appendmakeflags("-i");
		break;
	case 'k':
		kflag = 1;
		appendmakeflags("-k");
		break;
	case 'n':
		nflag = 1;
		appendmakeflags("-n");
		break;
	case 'p':
		pflag = 1;
		break;
	case 'q':
		qflag = 1;
		appendmakeflags("-q");
		break;
	case 'r':
		addtarget(".SUFFIXES", 0);
		appendmakeflags("-r");
		break;	
	case 'S':
		kflag = 0;
		appendmakeflags("-S");
		break;
	case 's':
		sflag = 1;
		appendmakeflags("-s");
		break;
	case 't':
		tflag = 1;
		appendmakeflags("-t");
		break;
	case 'd':
		dflag = 1;
		appendmakeflags("-d");
		break;
	default:
		usage();
	}
}

static int
assign(char *s, int where, int export)
{
	int pos;
	char *t;

	if ((t = strchr(s, '=')) == NULL)
		return 0;

	pos = t - s;

	appendmakeflags(s);
	t = estrdup(s); 
	t[pos] = '\0';

	setmacro(t, t+pos+1, where, export);
	free(t);
	return 1;
}

static void
parseargv(char **argv, char ***targets, int where, int export)
{
	char *s;
	char *hm = NULL;

	for ( ; *argv; ++argv) {
		s = *argv;
		if (hm == NULL && strcmp(s, "--") == 0) {
			hm = *argv;
		} else if (hm && s[0] == '-') {
			break;
		} else if (s[0] != '-') {
			if (!assign(s, where, export))
				break;
			continue;
		}
		while (hm == NULL && *++s)
			parseflag(*s, &s, &argv);
	}

	if (targets)
		*targets = argv;
}

static void
parsemakeflags(void)
{
	int c, n;
	char *s, *flags, **arr;

	if ((flags = getenv("MAKEFLAGS")) == NULL)
		return;

	setmacro("MAKEFLAGS", "", MAKEFLAGS, EXPORT);

	while (*flags == ' ' || *flags == '\t')
		flags++;

	if (flags[0] != '-' && !strchr(flags, '=')) {
		while (*flags) {
			parseflag(*flags, &flags, NULL);
			flags++;
		}
	} else {
		n = 0;
		arr = NULL;
		for (s = strtok(flags, " \t"); s; s = strtok(NULL, " \t")) {
			n++;
			arr = erealloc(arr, sizeof(char *) * (n+1));
			arr[n-1] = s;
			arr[n] = NULL;
		}

		if (arr)
			parseargv(arr, NULL, MAKEFLAGS, NOEXPORT);
		free(arr);
	}
}

static void
parsemakefiles(char **argv)
{
	char *s, *arg;
	int c, hasmake;

	hasmake = 0;
	for ( ; *argv && **argv == '-'; ++argv) {
		for (s = *argv; c = *s; ++s) {
			if (hasargs(c))
				arg = getarg(&s, &argv);

			if (c == 'f') {
				if (strcmp(arg, "-") == 0)
					arg = NULL;
				parse(arg);
				hasmake = 1;
			}
		}
	}

	if (hasmake)
		return;

	if (parse("makefile"))
		return;
	if (parse("Makefile"))
		return;
}

/*
 * We want to enable debug as earlier as possible,
 * if we wait until we read the Makefiles then
 * we are going to lose to much debug information.
 */
static void
enadebug(char *argv[])
{
	int c;
	char *p;

	for ( ; *argv && **argv == '-'; ++argv) {
		p = *argv;
		for (++p; c = *p; ++p) {
			if (hasargs(c))
				getarg(&p, &argv);
			if (c == 'd')
				dflag = 1;
		}
	}
}

int
main(int argc, char *argv[])
{
	char *arg0, **targets;

	signal(SIGINT, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGQUIT, sighandler);

	targets = NULL;
	arg0 = *argv++;

	enadebug(argv);
	inject(defaults);
	setmacro("MAKE", arg0, MAKEFILE, NOEXPORT);

	parsemakeflags();
	parseargv(argv, &targets, CMDLINE, EXPORT);
	parsemakefiles(argv);

	if (pflag) {
		dumpmacros();
		dumprules();
		exit(EXIT_SUCCESS);
	}

	if (!*targets) {
		build(NULL);
	} else {
		while (*targets)
			build(*targets++);
	}

	exit(exitstatus);

	return 0;
}
