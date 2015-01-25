/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static int bflag = 0;
static int sflag = 0;

static void
foldline(const char *str, size_t width)
{
	int space;
	size_t i = 0, n = 0, col, j;
	char c;

	do {
		space = 0;
		for (j = i, col = 0; str[j] && col <= width; j++) {
			c = str[j];
			if (!UTF8_POINT(c) && !bflag)
				continue;
			if (sflag && isspace(c)) {
				space = 1;
				n = j + 1;
			}
			else if (!space)
				n = j;

			if (!bflag && iscntrl(c))
				switch(c) {
				case '\b':
					col--;
					break;
				case '\r':
					col = 0;
					break;
				case '\t':
					col += (col + 1) % 8;
					break;
				}
			else
				col++;
		}
		if (fwrite(&str[i], 1, n - i, stdout) != n - i)
			eprintf("<stdout>: write error:");
		if (str[n])
			putchar('\n');
	} while (str[i = n] && str[i] != '\n');
}

static void
fold(FILE *fp, long width)
{
	char *buf = NULL;
	size_t size = 0;

	while (getline(&buf, &size, fp) != -1)
		foldline(buf, width);
	free(buf);
}

static void
usage(void)
{
	eprintf("usage: %s [-bs] [-w width] [-N] [FILE...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	size_t width = 80;
	FILE *fp;

	ARGBEGIN {
	case 'b':
		bflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'w':
		width = estrtol(EARGF(usage()), 0);
		break;
	ARGNUM:
		width = ARGNUMF(0);
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		fold(stdin, width);
	else {
		for (; argc > 0; argc--, argv++) {
			if (!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				continue;
			}
			fold(fp, width);
			fclose(fp);
		}
	}

	return 0;
}
