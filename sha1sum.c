/* See LICENSE file for copyright and license details. */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"
#include "sha1.h"

static void sha1sum(int fd, const char *f);

static void
usage(void)
{
	eprintf("usage: %s [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int fd;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		sha1sum(STDIN_FILENO, "<stdin>");
	} else {
		for (; argc > 0; argc--) {
			if ((fd = open(*argv, O_RDONLY)) < 0)
				eprintf("open %s:", *argv);
			sha1sum(fd, *argv);
			close(fd);
			argv++;
		}
	}

	return 0;
}

static void
sha1sum(int fd, const char *f)
{
	unsigned char buf[BUFSIZ];
	unsigned char digest[SHA1_DIGEST_LENGTH];
	struct sha1 s;
	ssize_t n;
	int i;

	sha1_init(&s);
	while ((n = read(fd, buf, sizeof buf)) > 0)
		sha1_update(&s, buf, n);
	if (n < 0) {
		eprintf("%s: read error:", f);
		return;
	}

	sha1_sum(&s, digest);

	for (i = 0; i < sizeof(digest); i++)
		printf("%02x", digest[i]);
	printf("  %s\n", f);
}
