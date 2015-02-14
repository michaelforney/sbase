/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <string.h>
#include <unistd.h>

#include "utf.h"
#include "util.h"

#define STOI(s) enstrtonum(2, s, LLONG_MIN, LLONG_MAX)

static int unary_b(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISBLK  (buf.st_mode); }
static int unary_c(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISCHR  (buf.st_mode); }
static int unary_d(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISDIR  (buf.st_mode); }
static int unary_f(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISREG  (buf.st_mode); }
static int unary_g(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISGID & buf.st_mode ; }
static int unary_h(char *s) { struct stat buf; if (lstat(s, &buf)) return 0; return S_ISLNK  (buf.st_mode); }
static int unary_p(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISFIFO (buf.st_mode); }
static int unary_S(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISSOCK (buf.st_mode); }
static int unary_s(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return           buf.st_size ; }
static int unary_u(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISUID & buf.st_mode ; }

static int unary_n(char *s) { return  utflen(s); }
static int unary_z(char *s) { return !utflen(s); }

static int unary_e(char *s) { return access(s, F_OK); }
static int unary_r(char *s) { return access(s, R_OK); }
static int unary_w(char *s) { return access(s, W_OK); }
static int unary_x(char *s) { return access(s, X_OK); }

static int unary_t(char *s) { int fd = enstrtonum(2, s, 0, INT_MAX); return isatty(fd); }

static int binary_se(char *s1, char *s2) { return strcmp(s1, s2) == 0; }
static int binary_sn(char *s1, char *s2) { return strcmp(s1, s2) != 0; }

static int binary_eq(char *s1, char *s2) { long long a = STOI(s1), b = STOI(s2); return a == b; }
static int binary_ne(char *s1, char *s2) { long long a = STOI(s1), b = STOI(s2); return a != b; }
static int binary_gt(char *s1, char *s2) { long long a = STOI(s1), b = STOI(s2); return a >  b; }
static int binary_ge(char *s1, char *s2) { long long a = STOI(s1), b = STOI(s2); return a >= b; }
static int binary_lt(char *s1, char *s2) { long long a = STOI(s1), b = STOI(s2); return a <  b; }
static int binary_le(char *s1, char *s2) { long long a = STOI(s1), b = STOI(s2); return a <= b; }

struct test {
	char *name;
	int (*func)();
};

static struct test unary[] = {
	{ "-b", unary_b },
	{ "-c", unary_c },
	{ "-d", unary_d },
	{ "-e", unary_e },
	{ "-f", unary_f },
	{ "-g", unary_g },
	{ "-h", unary_h },
	{ "-L", unary_h },
	{ "-n", unary_n },
	{ "-p", unary_p },
	{ "-r", unary_r },
	{ "-S", unary_S },
	{ "-s", unary_s },
	{ "-t", unary_t },
	{ "-u", unary_u },
	{ "-w", unary_w },
	{ "-x", unary_x },
	{ "-z", unary_z },

	{ NULL, NULL },
};

static struct test binary[] = {
	{ "="  , binary_se },
	{ "!=" , binary_sn },
	{ "-eq", binary_eq },
	{ "-ne", binary_ne },
	{ "-gt", binary_gt },
	{ "-ge", binary_ge },
	{ "-lt", binary_lt },
	{ "-le", binary_le },

	{ NULL, NULL },
};

static struct test *
find_test(struct test *tests, char *name)
{
	struct test *t;

	for (t = tests; t->name; ++t)
		if (strcmp(t->name, name) == 0)
			return t;
	return NULL;
}

static int
noarg(char **argv)
{
	return 0;
}

static int
onearg(char **argv)
{
	return strlen(argv[0]);
}

static int
twoarg(char **argv)
{
	struct test *t = find_test(unary, *argv);

	if (strcmp(argv[0], "!") == 0)
		return !onearg(argv + 1);

	if (t)
		return t->func(argv[1]);

	return enprintf(2, "bad unary test %s\n", argv[0]), 0;
}

static int
threearg(char **argv)
{
	struct test *t = find_test(binary, argv[1]);

	if (t)
		return t->func(argv[0], argv[2]);

	if (strcmp(argv[0], "!") == 0)
		return !twoarg(argv + 1);

	return enprintf(2, "bad binary test %s\n", argv[1]), 0;
}

static int
fourarg(char **argv)
{
	if (strcmp(argv[0], "!") == 0)
		return !threearg(argv + 1);

	return enprintf(2, "too many arguments\n"), 0;
}

int
main(int argc, char **argv)
{
	int (*narg[])(char**) = { noarg, onearg, twoarg, threearg, fourarg };
	size_t len = strlen(argv[0]);

	if (len && argv[0][len - 1] == '[' && strcmp(argv[--argc], "]") != 0)
		enprintf(2, "no matching ]\n");

	--argc; ++argv;

	if (argc > 4)
		enprintf(2, "too many arguments\n");

	return !narg[argc](argv);
}
