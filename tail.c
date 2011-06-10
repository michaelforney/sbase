/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

static void dropinit(FILE *, const char *, long);
static void taketail(FILE *, const char *, long);

int
main(int argc, char *argv[])
{
	char c;
	long n = 10;
	FILE *fp;
	void (*tail)(FILE *, const char *, long) = taketail;

	while((c = getopt(argc, argv, "n:")) != -1)
		switch(c) {
		case 'n':
			n = abs(estrtol(optarg, 0));
			if(optarg[0] == '+')
				tail = dropinit;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc)
		tail(stdin, "<stdin>", n);
	else if(optind == argc-1) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		tail(fp, argv[optind], n);
		fclose(fp);
	}
	else
		eprintf("usage: %s [-n lines] [file]\n", argv[0]);

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
