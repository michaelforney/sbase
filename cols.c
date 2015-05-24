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

static void
usage(void)
{
	eprintf("usage: %s [-c num] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	struct winsize w;
	struct linebuf b = EMPTY_LINEBUF;
	size_t chars = 65, maxlen = 0, i, j, k, len, bytes, cols, rows;
	int cflag = 0, ret = 0;
	char *p;

	ARGBEGIN {
	case 'c':
		cflag = 1;
		chars = estrtonum(EARGF(usage()), 1, MIN(LLONG_MAX, SIZE_MAX));
		break;
	default:
		usage();
	} ARGEND;

	if (!cflag) {
		if ((p = getenv("COLUMNS")))
			chars = estrtonum(p, 1, MIN(LLONG_MAX, SIZE_MAX));
		else if (!ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) && w.ws_col > 0)
			chars = w.ws_col;
	}

	if (!argc) {
		getlines(stdin, &b);
	} else {
		for (; *argv; argc--, argv++) {
			if (!strcmp(*argv, "-")) {
				*argv = "<stdin>";
				fp = stdin;
			} else if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			getlines(fp, &b);
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
		}
	}

	for (i = 0; i < b.nlines; i++) {
		len = utflen(b.lines[i]);
		bytes = strlen(b.lines[i]);
		if (len && bytes && b.lines[i][bytes - 1] == '\n') {
			b.lines[i][bytes - 1] = '\0';
			len--;
		}
		if (len > maxlen)
			maxlen = len;
	}

	for (cols = 1; (cols + 1) * maxlen + cols <= chars; cols++);
	rows = b.nlines / cols + (b.nlines % cols > 0);

	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols && i + j * rows < b.nlines; j++) {
			len = utflen(b.lines[i + j * rows]);
			fputs(b.lines[i + j * rows], stdout);
			if (j < cols - 1)
				for (k = len; k < maxlen + 1; k++)
					putchar(' ');
		}
		putchar('\n');
	}

	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
