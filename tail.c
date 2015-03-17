/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "utf.h"
#include "util.h"

static int    fflag = 0;
static size_t num   = 10;
static char   mode  = 'n';

static void
dropinit(FILE *fp, const char *str)
{
	Rune r;
	char *buf = NULL;
	size_t size = 0, i = 0;
	ssize_t len;

	if (mode == 'n') {
		while (i < num && (len = getline(&buf, &size, fp)) >= 0)
			if (len > 0 && buf[len - 1] == '\n')
				i++;
	} else {
		while (i < num && (len = efgetrune(&r, fp, str)))
			i++;
	}
	free(buf);
	concat(fp, str, stdout, "<stdout>");
}

static void
taketail(FILE *fp, const char *str)
{
	Rune *r = NULL;
	char **ring = NULL;
	size_t i, j, *size = NULL;

	if (mode == 'n') {
		ring = ecalloc(num, sizeof *ring);
		size = ecalloc(num, sizeof *size);

		for (i = j = 0; getline(&ring[i], &size[i], fp) != -1; )
			i = j = (i + 1) % num;
	} else {
		r = ecalloc(num, sizeof *r);

		for (i = j = 0; efgetrune(&r[i], fp, str); )
			i = j = (i + 1) % num;
	}
	if (ferror(fp))
		eprintf("%s: read error:", str);

	do {
		if (ring && ring[j]) {
			fputs(ring[j], stdout);
			free(ring[j]);
		} else if (r) {
			efputrune(&r[j], stdout, "<stdout>");
		}
	} while ((j = (j + 1) % num) != i);

	free(ring);
	free(size);
	free(r);
}

static void
usage(void)
{
	eprintf("usage: %s [-f] [-c num | -n num | -num] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct stat st1, st2;
	FILE *fp;
	size_t tmpsize;
	int ret = 0, newline, many;
	char *numstr, *tmp;
	void (*tail)(FILE *, const char *) = taketail;

	ARGBEGIN {
	case 'f':
		fflag = 1;
		break;
	case 'c':
	case 'n':
		mode = ARGC();
		numstr = EARGF(usage());
		num = MIN(llabs(estrtonum(numstr, LLONG_MIN + 1, MIN(LLONG_MAX, SIZE_MAX))), SIZE_MAX);
		if (strchr(numstr, '+'))
			tail = dropinit;
		break;
	ARGNUM:
		num = ARGNUMF();
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		tail(stdin, "<stdin>");
	else {
		if ((many = argc > 1) && fflag)
			usage();
		for (newline = 0; *argv; argc--, argv++) {
			if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			if (many)
				printf("%s==> %s <==\n", newline ? "\n" : "", *argv);
			if (stat(*argv, &st1) < 0)
				eprintf("stat %s:", *argv);
			if (!(S_ISFIFO(st1.st_mode) || S_ISREG(st1.st_mode)))
				fflag = 0;
			newline = 1;
			tail(fp, *argv);

			if (!fflag) {
				fclose(fp);
				continue;
			}
			for (tmp = NULL, tmpsize = 0;;) {
				while (getline(&tmp, &tmpsize, fp) >= 0) {
					fputs(tmp, stdout);
					fflush(stdout);
				}
				if (ferror(fp))
					eprintf("readline %s:", *argv);
				clearerr(fp);
				/* ignore error in case file was removed, we continue
				 * tracking the existing open file descriptor */
				if (!stat(*argv, &st2)) {
					if (st2.st_size < st1.st_size) {
						fprintf(stderr, "%s: file truncated\n", *argv);
						rewind(fp);
					}
					st1 = st2;
				}
				sleep(1);
			}
		}
	}

	return ret;
}
