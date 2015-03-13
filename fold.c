/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static int    bflag = 0;
static int    sflag = 0;
static size_t width = 80;

static void
foldline(const char *str)
{
	size_t i = 0, n = 0, col, j;
	int space;
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
			} else if (!space) {
				n = j;
			}

			if (!bflag && iscntrl(c)) {
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
			} else {
				col++;
			}
		}
		if (fwrite(str + i, 1, n - i, stdout) != n - i)
			eprintf("fwrite <stdout>:");
		if (str[n])
			putchar('\n');
	} while (str[i = n] && str[i] != '\n');
}

static void
fold(FILE *fp, const char *fname)
{
	char *buf = NULL;
	size_t size = 0;

	while (getline(&buf, &size, fp) >= 0)
		foldline(buf);
	if (ferror(fp))
		eprintf("getline %s:", fname);
	free(buf);
}

static void
usage(void)
{
	eprintf("usage: %s [-bs] [-w width | -width] [FILE...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;

	ARGBEGIN {
	case 'b':
		bflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'w':
		width = estrtonum(EARGF(usage()), 1, MIN(LLONG_MAX, SIZE_MAX));
		break;
	ARGNUM:
		width = ARGNUMF();
		break;
	default:
		usage();
	} ARGEND;

	if (!argc) {
		fold(stdin, "<stdin>");
	} else {
		for (; *argv; argc--, argv++) {
			if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			fold(fp, *argv);
			fclose(fp);
		}
	}

	return ret;
}
