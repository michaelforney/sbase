/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void
uconcat(FILE *fp1, const char *s1, FILE *fp2, const char *s2)
{
	int c;

	setbuf(fp2, NULL);
	while ((c = getc(fp1)) != EOF)
		putc(c, fp2);
}

static void
usage(void)
{
	eprintf("usage: %s [-u] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;
	void (*cat)(FILE *, const char *, FILE *, const char *) = &concat;

	ARGBEGIN {
	case 'u':
		cat = &uconcat;
		break;
	default:
		usage();
	} ARGEND

	if (!argc) {
		cat(stdin, "<stdin>", stdout, "<stdout>");
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
			cat(fp, *argv, stdout, "<stdout>");
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
		}
	}

	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
