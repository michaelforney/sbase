/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static void head(FILE *, const char *, long);

int
main(int argc, char *argv[])
{
	char c;
	long n = 10;
	FILE *fp;

	while((c = getopt(argc, argv, "n:")) != -1)
		switch(c) {
		case 'n':
			n = estrtol(optarg, 0);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc)
		head(stdin, "<stdin>", n);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		head(fp, argv[optind], n);
		fclose(fp);
	}
	return EXIT_SUCCESS;
}

void
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
