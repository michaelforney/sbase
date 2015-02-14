/* See LICENSE file for copyright and license details. */
#include <sys/ioctl.h>

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "utf.h"
#include "util.h"

static size_t chars = 65;
static int    cflag;
static struct linebuf b = EMPTY_LINEBUF;

static size_t n_columns;
static size_t n_rows;

static void
usage(void)
{
	eprintf("usage: %s [-c chars] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	size_t i, l, col, len, bytes, maxlen = 0;
	struct winsize w;
	FILE *fp;

	ARGBEGIN {
	case 'c':
		cflag = 1;
		chars = estrtonum(EARGF(usage()), 3, MIN(LLONG_MAX, SIZE_MAX));
		break;
	default:
		usage();
	} ARGEND;

	if (cflag == 0) {
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		if (w.ws_col != 0)
			chars = w.ws_col;
	}

	if (argc == 0) {
		getlines(stdin, &b);
	} else for (; argc > 0; argc--, argv++) {
		if (!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		getlines(fp, &b);
		fclose(fp);
	}

	for (l = 0; l < b.nlines; ++l) {
		len = utflen(b.lines[l]);
		bytes = strlen(b.lines[l]);
		if (len > 0 && b.lines[l][bytes - 1] == '\n') {
			b.lines[l][bytes - 1] = '\0';
			--len;
		}
		if (len > maxlen)
			maxlen = len;
		if (maxlen > (chars - 1) / 2)
			break;
	}

	n_columns = (chars + 1) / (maxlen + 1);
	if (n_columns <= 1) {
		for (l = 0; l < b.nlines; ++l) {
			fputs(b.lines[l], stdout);
		}
		return 0;
	}

	n_rows = (b.nlines + (n_columns - 1)) / n_columns;
	for (i = 0; i < n_rows; ++i) {
		for (l = i, col = 1; l < b.nlines; l += n_rows, ++col) {
			len = utflen(b.lines[l]);
			fputs(b.lines[l], stdout);
			if (col < n_columns)
				printf("%*s", (int)(maxlen + 1 - len), "");
		}
		fputs("\n", stdout);
	}

	return 0;
}
