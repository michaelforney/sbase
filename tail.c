/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void dropinit(FILE *, const char *, long);
static void taketail(FILE *, const char *, long);

static void
usage(void)
{
	eprintf("usage: %s [-n lines] [file]\n", argv0);
}

int
main(int argc, char *argv[])
{
	long n = 10;
	FILE *fp;
	void (*tail)(FILE *, const char *, long) = taketail;
	char *lines;
	int ret = 0;

	ARGBEGIN {
	case 'n':
		lines = EARGF(usage());
		n = abs(estrtol(lines, 0));
		if (lines[0] == '+')
			tail = dropinit;
		break;
	ARGNUM:
		n = ARGNUMF(0);
		break;
	default:
		usage();
	} ARGEND;
	if (argc == 0) {
		tail(stdin, "<stdin>", n);
	} else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				ret = 1;
				continue;
			}
			tail(fp, argv[0], n);
			fclose(fp);
		}
	}
	return ret;
}

static void
dropinit(FILE *fp, const char *str, long n)
{
	char *buf = NULL;
	size_t size = 0;
	ssize_t len;
	unsigned long i = 0;

	while (i < n && ((len = getline(&buf, &size, fp)) != -1))
		if (len && buf[len - 1] == '\n')
			i++;
	free(buf);
	concat(fp, str, stdout, "<stdout>");
}

static void
taketail(FILE *fp, const char *str, long n)
{
	char **ring = NULL;
	long i, j;
	size_t *size = NULL;

	ring = ecalloc(n, sizeof *ring);
	size = ecalloc(n, sizeof *size);

	for (i = j = 0; getline(&ring[i], &size[i], fp) != -1; i = j = (i + 1) % n)
		;
	if (ferror(fp))
		eprintf("%s: read error:", str);

	do {
		if (ring[j]) {
			fputs(ring[j], stdout);
			free(ring[j]);
		}
	} while ((j = (j+1)%n) != i);
	free(ring);
	free(size);
}
