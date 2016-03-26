/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static int
intcmp(char *a, char *b)
{
	char *s;
	int asign = *a == '-' ? -1 : 1;
	int bsign = *b == '-' ? -1 : 1;

	if (*a == '-' || *a == '+') a += 1;
	if (*b == '-' || *b == '+') b += 1;

	if (!*a || !*b)
		goto noint;
	for (s = a; *s; s++)
		if (!isdigit(*s))
			goto noint;
	for (s = b; *s; s++)
		if (!isdigit(*s))
			goto noint;

	while (*a == '0') a++;
	while (*b == '0') b++;
	asign *= !!*a;
	bsign *= !!*b;

	if (asign != bsign)
		return asign < bsign ? -1 : 1;
	else if (strlen(a) != strlen(b))
		return asign * (strlen(a) < strlen(b) ? -1 : 1);
	else
		return asign * strcmp(a, b);

noint:
	enprintf(2, "expected integer operands\n");

	return 0; /* not reached */
}

static int
mtimecmp(struct stat *buf1, struct stat *buf2)
{
	if (buf1->st_mtime < buf2->st_mtime) return -1;
	if (buf1->st_mtime > buf2->st_mtime) return +1;
#ifdef st_mtime
	if (buf1->st_mtim.tv_nsec < buf2->st_mtim.tv_nsec) return -1;
	if (buf1->st_mtim.tv_nsec > buf2->st_mtim.tv_nsec) return +1;
#endif
	return 0;
}

static int unary_b(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISBLK  (buf.st_mode); }
static int unary_c(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISCHR  (buf.st_mode); }
static int unary_d(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISDIR  (buf.st_mode); }
static int unary_f(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISREG  (buf.st_mode); }
static int unary_g(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISGID & buf.st_mode ; }
static int unary_h(char *s) { struct stat buf; if (lstat(s, &buf)) return 0; return S_ISLNK  (buf.st_mode); }
static int unary_k(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISVTX & buf.st_mode ; }
static int unary_p(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISFIFO (buf.st_mode); }
static int unary_S(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISSOCK (buf.st_mode); }
static int unary_s(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return           buf.st_size ; }
static int unary_u(char *s) { struct stat buf; if ( stat(s, &buf)) return 0; return S_ISUID & buf.st_mode ; }

static int unary_n(char *s) { return  *s; }
static int unary_z(char *s) { return !*s; }

static int unary_e(char *s) { return !access(s, F_OK); }
static int unary_r(char *s) { return !access(s, R_OK); }
static int unary_w(char *s) { return !access(s, W_OK); }
static int unary_x(char *s) { return !access(s, X_OK); }

static int unary_t(char *s) { int fd = enstrtonum(2, s, 0, INT_MAX); return isatty(fd); }

static int binary_se(char *s1, char *s2) { return !strcmp(s1, s2); }
static int binary_sn(char *s1, char *s2) { return  strcmp(s1, s2); }

static int binary_eq(char *s1, char *s2) { return intcmp(s1, s2) == 0; }
static int binary_ne(char *s1, char *s2) { return intcmp(s1, s2) != 0; }
static int binary_gt(char *s1, char *s2) { return intcmp(s1, s2) >  0; }
static int binary_ge(char *s1, char *s2) { return intcmp(s1, s2) >= 0; }
static int binary_lt(char *s1, char *s2) { return intcmp(s1, s2) <  0; }
static int binary_le(char *s1, char *s2) { return intcmp(s1, s2) <= 0; }

static int
binary_ef(char *s1, char *s2)
{
	struct stat buf1, buf2;
	if (stat(s1, &buf1) || stat(s2, &buf2)) return 0;
	return buf1.st_dev == buf2.st_dev && buf1.st_ino == buf2.st_ino;
}

static int
binary_ot(char *s1, char *s2)
{
	struct stat buf1, buf2;
	if (stat(s1, &buf1) || stat(s2, &buf2)) return 0;
	return mtimecmp(&buf1, &buf2) < 0;
}

static int
binary_nt(char *s1, char *s2)
{
	struct stat buf1, buf2;
	if (stat(s1, &buf1) || stat(s2, &buf2)) return 0;
	return mtimecmp(&buf1, &buf2) > 0;
}

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
	{ "-k", unary_k },
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
	{ "-ef", binary_ef },
	{ "-ot", binary_ot },
	{ "-nt", binary_nt },

	{ NULL, NULL },
};

static struct test *
find_test(struct test *tests, char *name)
{
	struct test *t;

	for (t = tests; t->name; t++)
		if (!strcmp(t->name, name))
			return t;

	return NULL;
}

static int
noarg(char *argv[])
{
	return 0;
}

static int
onearg(char *argv[])
{
	return unary_n(argv[0]);
}

static int
twoarg(char *argv[])
{
	struct test *t;

	if (!strcmp(argv[0], "!"))
		return !onearg(argv + 1);

	if ((t = find_test(unary, *argv)))
		return t->func(argv[1]);

	enprintf(2, "bad unary test %s\n", argv[0]);

	return 0; /* not reached */
}

static int
threearg(char *argv[])
{
	struct test *t = find_test(binary, argv[1]);

	if (t)
		return t->func(argv[0], argv[2]);

	if (!strcmp(argv[0], "!"))
		return !twoarg(argv + 1);

	enprintf(2, "bad binary test %s\n", argv[1]);

	return 0; /* not reached */
}

static int
fourarg(char *argv[])
{
	if (!strcmp(argv[0], "!"))
		return !threearg(argv + 1);

	enprintf(2, "too many arguments\n");

	return 0; /* not reached */
}

int
main(int argc, char *argv[])
{
	int (*narg[])(char *[]) = { noarg, onearg, twoarg, threearg, fourarg };
	size_t len;

	argv0 = argv[0], argc--, argv++;

	len = strlen(argv0);
	if (len && argv0[--len] == '[' && (!len || argv0[--len] == '/') && strcmp(argv[--argc], "]"))
		enprintf(2, "no matching ]\n");

	if (argc > 4)
		enprintf(2, "too many arguments\n");

	return !narg[argc](argv);
}
