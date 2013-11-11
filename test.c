/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static bool unary(const char *, const char *);
static bool binary(const char *, const char *, const char *);
static void usage(void);

int
main(int argc, char *argv[])
{
	bool ret = false, not = false;

	argv0 = argv[0];

	/* [ ... ] alias */
	if(!strcmp(argv[0], "[")) {
		if(strcmp(argv[argc-1], "]") != 0)
			usage();
		argc--;
	}
	if(argc > 1 && !strcmp(argv[1], "!")) {
		not = true;
		argv++;
		argc--;
	}
	switch(argc) {
	case 2:
		ret = *argv[1] != '\0';
		break;
	case 3:
		ret = unary(argv[1], argv[2]);
		break;
	case 4:
		ret = binary(argv[1], argv[2], argv[3]);
		break;
	default:
		usage();
	}
	if(not)
		ret = !ret;
	return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool
unary(const char *op, const char *arg)
{
	struct stat st;
	int r;

	if(op[0] != '-' || op[1] == '\0' || op[2] != '\0')
		usage();
	switch(op[1]) {
	case 'b': case 'c': case 'd': case 'f': case 'g':
	case 'k': case 'p': case 'S': case 's': case 'u':
		if((r = stat(arg, &st)) == -1)
			return false; /* -e */
		switch(op[1]) {
		case 'b':
			return S_ISBLK(st.st_mode);
		case 'c':
			return S_ISCHR(st.st_mode);
		case 'd':
			return S_ISDIR(st.st_mode);
		case 'f':
			return S_ISREG(st.st_mode);
		case 'g':
			return st.st_mode & S_ISGID;
		case 'k':
			return st.st_mode & S_ISVTX;
		case 'p':
			return S_ISFIFO(st.st_mode);
		case 'S':
			return S_ISSOCK(st.st_mode);
		case 's':
			return st.st_size > 0;
		case 'u':
			return st.st_mode & S_ISUID;
		}
	case 'e':
		return access(arg, F_OK) == 0;
	case 'r':
		return access(arg, R_OK) == 0;
	case 'w':
		return access(arg, W_OK) == 0;
	case 'x':
		return access(arg, X_OK) == 0;
	case 'h': case 'L':
		return lstat(arg, &st) == 0 && S_ISLNK(st.st_mode);
	case 't':
		return isatty((int)estrtol(arg, 0));
	case 'n':
		return arg[0] != '\0';
	case 'z':
		return arg[0] == '\0';
	default:
		usage();
	}
	return false; /* should not reach */
}

bool
binary(const char *arg1, const char *op, const char *arg2)
{
	int i;
	long narg1, narg2;
	enum operator { EQ, GE, GT, LE, LT, NE, STREQ, STRNE } oper;
	char *optexts[] = {
		"-eq", "-ge", "-gt",
		"-le", "-lt", "-ne",
		"=", "!="
	};

	for (i = 0; i < LEN(optexts); i++) {
		if (strcmp(op, optexts[i]) != 0)
			continue;
		oper = i;
		switch (oper) {
		case STREQ:
			return strcmp(arg1, arg2) == 0;
		case STRNE:
			return strcmp(arg1, arg2) != 0;
		default:
			narg1 = estrtol(arg1, 0);
			narg2 = estrtol(arg2, 0);
			switch (oper) {
			case EQ:
				return narg1 == narg2;
			case GE:
				return narg1 >= narg2;
			case GT:
				return narg1 > narg2;
			case LE:
				return narg1 <= narg2;
			case LT:
				return narg1 < narg2;
			case NE:
				return narg1 != narg2;
			default:
				usage();
			}
		}
	}
	return false;
}

void
usage(void)
{
	const char *ket = (*argv0 == '[') ? " ]" : "";

	eprintf("usage: %s string%s\n"
	        "       %s [!] [-bcdefgkhLnprSstuwxz] string%s\n", argv0, ket, argv0, ket);
}
