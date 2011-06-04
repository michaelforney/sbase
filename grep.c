/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "text.h"

static void grep(FILE *, const char *, regex_t *);

static bool Eflag = false;
static bool iflag = false;
static bool vflag = false;
static bool many;
static bool match = false;
static char mode = 0;

int
main(int argc, char *argv[])
{
	char c;
	int flags = REG_NOSUB;
	regex_t preg;
	FILE *fp;

	while((c = getopt(argc, argv, "Ecilnqv")) != -1)
		switch(c) {
		case 'E':
			Eflag = true;
			break;
		case 'c':
		case 'l':
		case 'n':
		case 'q':
			mode = c;
			break;
		case 'i':
			iflag = true;
			break;
		case 'v':
			vflag = true;
			break;
		default:
			exit(2);
		}
	if(optind == argc) {
		fprintf(stderr, "usage: %s [-cilnqv] pattern [files...]\n", argv[0]);
		exit(2);
	}
	if(Eflag)
		flags |= REG_EXTENDED;
	if(iflag)
		flags |= REG_ICASE;
	regcomp(&preg, argv[optind++], flags);

	many = (argc > optind+1);
	if(optind == argc)
		grep(stdin, "<stdin>", &preg);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r"))) {
			fprintf(stderr, "fopen %s: ", argv[optind]);
			perror(NULL);
			exit(2);
		}
		grep(fp, argv[optind], &preg);
		fclose(fp);
	}
	return match ? 0 : 1;
}

void
grep(FILE *fp, const char *str, regex_t *preg)
{
	char *buf = NULL;
	long n, c = 0;
	size_t size = 0;

	for(n = 1; afgets(&buf, &size, fp); n++) {
		if(regexec(preg, buf, 0, NULL, 0) ^ vflag)
			continue;
		switch(mode) {
		case 'c':
			c++;
			break;
		case 'l':
			puts(str);
			goto end;
		case 'q':
			exit(0);
		default:
			if(many)
				printf("%s:", str);
			if(mode == 'n')
				printf("%ld:", n);
			fputs(buf, stdout);
			break;
		}
		match = true;
	}
	if(mode == 'c')
		printf("%ld\n", c);
end:
	if(ferror(fp)) {
		fprintf(stderr, "%s: read error: ", str);
		perror(NULL);
		exit(2);
	}
	free(buf);
}
