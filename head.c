/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

	return EXIT_SUCCESS;
}

static void
head(FILE *fp, const char *str, long n)
{
	char buf[BUFSIZ];
	long i = 0;

	while(i < n && fgets(buf, sizeof buf, fp)) {
		fputs(buf, stdout);
		if(buf[strlen(buf)-1] == '\n')
			i++;
	}
	if(ferror(fp))
		eprintf("%s: read error:", str);
}
