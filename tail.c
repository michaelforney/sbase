/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static int fflag = 0;

static void
dropinit(FILE *fp, const char *str, long n)
{
	char *buf = NULL;
	size_t size = 0;
	ssize_t len;
	unsigned long i = 0;

	while (i < n && (len = getline(&buf, &size, fp)) != -1)
		if (len > 0 && buf[len - 1] == '\n')
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
	} while ((j = (j + 1) % n) != i);

	free(ring);
	free(size);
}

static void
usage(void)
{
	eprintf("usage: %s [-f] [-n lines] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	size_t n = 10, tmpsize;
	int ret = 0, newline, many;
	char *lines, *tmp;
	void (*tail)(FILE *, const char *, long) = taketail;

	ARGBEGIN {
	case 'f':
		fflag = 1;
		break;
	case 'n':
		lines = EARGF(usage());
		n = estrtonum(lines, 0, LONG_MAX);
		if (lines[0] == '+')
			tail = dropinit;
		break;
	ARGNUM:
		n = ARGNUMF();
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		tail(stdin, "<stdin>", n);
	else {
		if ((many = argc > 1) && fflag)
			usage();
		for (newline = 0; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				ret = 1;
				continue;
			}
			if (many)
				printf("%s==> %s <==\n",
				       newline ? "\n" : "", argv[0]);
			newline = 1;
			tail(fp, argv[0], n);

			if (fflag && argc == 1) {
				for(;; tmp = NULL, tmpsize = 0) {
					while (getline(&tmp, &tmpsize, fp) != -1) {
						fputs(tmp, stdout);
						fflush(stdout);
						free(tmp);
						tmp = NULL;
						tmpsize = 0;
					}
					if (ferror(fp))
						eprintf("readline '%s':", argv[0]);
					clearerr(fp);
				}
			}
			fclose(fp);
		}
	}
	return ret;
}
