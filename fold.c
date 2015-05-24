/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static int    bflag = 0;
static int    sflag = 0;
static size_t width = 80;

static void
foldline(const char *str) {
	const char *p, *spacesect = NULL;
	size_t col, off;

	for (p = str, col = 0; *p && *p != '\n'; p++) {
		if (!UTF8_POINT(*p) && !bflag)
			continue;
		if (col >= width) {
			off = (sflag && spacesect) ? spacesect - str : p - str;
			if (fwrite(str, 1, off, stdout) != off)
				eprintf("fwrite <stdout>:");
			putchar('\n');
			spacesect = NULL;
			col = 0;
			p = str += off;
		}
		if (sflag && isspace(*p))
			spacesect = p + 1;
		if (!bflag && iscntrl(*p)) {
			switch(*p) {
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
	fputs(str, stdout);
}

static void
fold(FILE *fp, const char *fname)
{
	char *buf = NULL;
	size_t size = 0;

	while (getline(&buf, &size, fp) > 0)
		foldline(buf);
	if (ferror(fp))
		eprintf("getline %s:", fname);
	free(buf);
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
	} ARGEND;

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
