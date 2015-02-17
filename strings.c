/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
strings(FILE *fp, const char *fname, int len)
{
	unsigned char buf[BUFSIZ];
	int c, i = 0;
	off_t offset = 0;

	do {
		offset++;
		if (isprint(c = getc(fp)))
			buf[i++] = c;
		if ((!isprint(c) && i >= len) || i == sizeof(buf) - 1) {
			buf[i] = '\0';
			printf("%8ld: %s\n", (long)offset - i - 1, buf);
			i = 0;
		}
	} while (c != EOF);
	if (ferror(fp))
		eprintf("%s: read error:", fname);
}

static void
usage(void)
{
	eprintf("usage: %s [-a] [-n len] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;
	int len = 4;

	ARGBEGIN {
	case 'a':
		break;
	case 'n':
		len = estrtonum(EARGF(usage()), 1, INT_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		strings(stdin, "<stdin>", len);
	} else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				ret = 1;
				continue;
			}
			strings(fp, argv[0], len);
			fclose(fp);
		}
	}
	return ret;
}
