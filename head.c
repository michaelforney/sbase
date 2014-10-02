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
	eprintf("usage: %s [-n] [FILE...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	long n = 10;
	FILE *fp;

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

	if(argc == 0) {
		head(stdin, "<stdin>", n);
	} else {
		for(; argc > 0; argc--, argv++) {
			if(!(fp = fopen(argv[0], "r"))) {
				weprintf("fopen %s:", argv[0]);
				continue;
			}
			head(fp, argv[0], n);
			fclose(fp);
		}
	}

	return 0;
}

static void
head(FILE *fp, const char *str, long n)
{
	char *buf = NULL;
	size_t size = 0;
	ssize_t len;
	unsigned long i = 0;

	while(i < n && ((len = agetline(&buf, &size, fp)) != -1)) {
		fputs(buf, stdout);
		if(buf[len - 1] == '\n')
			i++;
	}
	free(buf);
	if(ferror(fp))
		eprintf("%s: read error:", str);
}
