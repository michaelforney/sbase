/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

static void dropinit(FILE *, const char *, long);
static void taketail(FILE *, const char *, long);

static void
usage(void)
{
	eprintf("usage: %s [-n lines] [file]\n", argv0);
}

int
main(int argc, char *argv[])
{
	long n = 10;
	FILE *fp;
	void (*tail)(FILE *, const char *, long) = taketail;
	char *lines;

	ARGBEGIN {
	case 'n':
		lines = EARGF(usage());
		n = abs(estrtol(lines, 0));
		if(lines[0] == '+')
			tail = dropinit;
		break;
	ARGNUM:
		n = ARGNUMF(0);
		break;
	default:
		usage();
	} ARGEND;
	if(argc == 0) {
		tail(stdin, "<stdin>", n);
	} else {
		for (; argc > 0; argc--, argv++) {
			if(!(fp = fopen(argv[0], "r"))) {
				fprintf(stderr, "fopen %s: %s\n", argv[0],
					strerror(errno));
				continue;
			}
			tail(fp, argv[0], n);
			fclose(fp);
		}
	}

	return EXIT_SUCCESS;
}

void
dropinit(FILE *fp, const char *str, long n)
{
	char buf[BUFSIZ];
	long i = 0;

	while(i < n && fgets(buf, sizeof buf, fp))
		if(buf[strlen(buf)-1] == '\n')
			i++;
	concat(fp, str, stdout, "<stdout>");
}

void
taketail(FILE *fp, const char *str, long n)
{
	char **ring;
	long i, j;
	size_t *size = NULL;

	if(!(ring = calloc(n, sizeof *ring)) || !(size = calloc(n, sizeof *size)))
		eprintf("calloc:");
	for(i = j = 0; afgets(&ring[i], &size[i], fp); i = j = (i+1)%n)
		;
	if(ferror(fp))
		eprintf("%s: read error:", str);

	do {
		if(ring[j]) {
			fputs(ring[j], stdout);
			free(ring[j]);
		}
	} while((j = (j+1)%n) != i);
	free(ring);
	free(size);
}
