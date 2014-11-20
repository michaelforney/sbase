/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void head(FILE *, const char *, long);

static void
usage(void)
{
	eprintf("usage: %s [-n lines] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	long n = 10;
	FILE *fp;
	int ret = 0;
	int newline, many;

	ARGBEGIN {
	case 'n':
		n = estrtol(EARGF(usage()), 0);
		break;
	ARGNUM:
		n = ARGNUMF(0);
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		head(stdin, "<stdin>", n);
	} else {
		many = argc > 1;
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
			head(fp, argv[0], n);
			fclose(fp);
		}
	}
	return ret;
}

static void
head(FILE *fp, const char *str, long n)
{
	char *buf = NULL;
	size_t size = 0;
	ssize_t len;
	unsigned long i = 0;

	while (i < n && ((len = getline(&buf, &size, fp)) != -1)) {
		fputs(buf, stdout);
		if (buf[len - 1] == '\n')
			i++;
	}
	free(buf);
	if (ferror(fp))
		eprintf("%s: read error:", str);
}
