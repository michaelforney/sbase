/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

#include "util.h"
#include "arg.h"

#define PORTABLE_CHARACTER_SET "0123456789._-qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
/* If your system supports more other characters, but not all non-NUL characters, define SYSTEM_CHARACTER_SET. */

#ifndef PATH_MAX
# define PATH_MAX SIZE_MAX
#endif
#ifndef NAME_MAX
# define NAME_MAX SIZE_MAX
#endif

static int most = 0;
static int extra = 0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-pP] filename...\n", argv0);
	exit(1);
}

static int
pathchk(char *filename)
{
	char *invalid, *invalid_end, *p, *q;
	const char *character_set;
	size_t len, maxlen;
	struct stat _attr;

	/* Empty? */
	if (extra && !*filename) {
		fprintf(stderr, "%s: empty filename\n", argv0);
		return 1;
	}

	/* Leading hyphen? */
	if (extra && ((*filename == '-') || strstr(filename, "/-"))) {
		fprintf(stderr, "%s: %s: leading '-' in component of filename\n", argv0, filename);
		return 1;
	}

	/* Nonportable character? */
#ifdef SYSTEM_CHARACTER_SET
	character_set = "/"SYSTEM_CHARACTER_SET;
#else
	character_set = 0;
#endif
	if (most)
		character_set = "/"PORTABLE_CHARACTER_SET;
	if (character_set && *(invalid = filename + strspn(filename, character_set))) {
		for (invalid_end = invalid + 1; *invalid_end & 0x80; invalid_end++);
		fprintf(stderr, "%s: %s: ", argv0, filename);
		*invalid_end = 0;
		fprintf(stderr, "nonportable character '%s'\n", invalid);
		return 1;
	}

	/* Symlink error? Non-searchable directory? */
	if (lstat(filename, &_attr) && errno != ENOENT) {
		/* lstat rather than stat, so that if filename is a bad symlink, but
		 * all parents are OK, no error will be detected. */
		fprintf(stderr, "%s: %s: %s\n", argv0, filename, strerror(errno));
		return 1;
	}

	/* Too long pathname? */
	maxlen = most ? _POSIX_PATH_MAX : PATH_MAX;
	if (strlen(filename) >= maxlen) {
		fprintf(stderr, "%s: %s: is longer than %zu bytes\n",
		        argv0, filename, maxlen);
		return 1;
	}

	/* Too long component? */
	maxlen = most ? _POSIX_NAME_MAX : NAME_MAX;
	for (p = filename; p; p = q) {
		q = strchr(p, '/');
		len = q ? (size_t)(q++ - p) : strlen(p);
		if (len > maxlen) {
			fprintf(stderr, "%s: %s: includes component longer than %zu bytes\n",
			        argv0, filename, maxlen);
			return 1;
		}
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int ret = 0;

	ARGBEGIN {
	case 'p':
		most = 1;
		break;
	case 'P':
		extra = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();

	for (; argc--; argv++)
		ret |= pathchk(*argv);

	return ret;
}
