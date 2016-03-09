/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"
#include "util.h"

static int    bflag = 0;
static int    sflag = 0;
static size_t width = 80;

static void
foldline(struct line *l) {
	size_t i, col, last, spacesect, len;

	for (i = 0, last = 0, col = 0, spacesect = 0; i < l->len; i++) {
		if (!UTF8_POINT(l->data[i]) && !bflag)
			continue;
		if (col >= width) {
			len = ((sflag && spacesect) ? spacesect : i) - last;
			if (fwrite(l->data + last, 1, len, stdout) != len)
				eprintf("fwrite <stdout>:");
			putchar('\n');
			last = (sflag && spacesect) ? spacesect : i;
			col = 0;
			spacesect = 0;
		}
		if (sflag && isspace(l->data[i]))
			spacesect = i + 1;
		if (!bflag && iscntrl(l->data[i])) {
			switch(l->data[i]) {
			case '\b':
				col -= (col > 0);
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
	if (l->len - last)
		fwrite(l->data + last, 1, l->len - last, stdout);
}

static void
fold(FILE *fp, const char *fname)
{
	static struct line line;
	static size_t size = 0;
	ssize_t len;

	while ((len = getline(&line.data, &size, fp)) > 0) {
		line.len = len;
		foldline(&line);
	}
	if (ferror(fp))
		eprintf("getline %s:", fname);
}

static void
usage(void)
{
	eprintf("usage: %s [-bs] [-w num | -num] [FILE ...]\n", argv0);
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
		if (!(width = ARGNUMF()))
			eprintf("illegal width value, too small\n");
		break;
	default:
		usage();
	} ARGEND

	if (!argc) {
		fold(stdin, "<stdin>");
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
			fold(fp, *argv);
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
		}
	}

	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
