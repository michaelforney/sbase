/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

enum { Match = 0, NoMatch = 1, Error = 2 };

static void grep(FILE *, const char *, regex_t *);

static bool vflag = false;
static bool many;
static bool match = false;
static char mode = 0;

int
main(int argc, char *argv[])
{
	char c;
	int n, flags = REG_NOSUB;
	regex_t preg;
	FILE *fp;

	while((c = getopt(argc, argv, "Ecilnqv")) != -1)
		switch(c) {
		case 'E':
			flags |= REG_EXTENDED;
			break;
		case 'c':
		case 'l':
		case 'n':
		case 'q':
			mode = c;
			break;
		case 'i':
			flags |= REG_ICASE;
			break;
		case 'v':
			vflag = true;
			break;
		default:
			exit(Error);
		}
	if(optind == argc)
		enprintf(Error, "usage: %s [-Ecilnqv] pattern [files...]\n", argv[0]);

	if((n = regcomp(&preg, argv[optind++], flags)) != 0) {
		char buf[BUFSIZ];

		regerror(n, &preg, buf, sizeof buf);
		enprintf(Error, "invalid pattern: %s\n", buf);
	}
	many = (argc > optind+1);
	if(optind == argc)
		grep(stdin, "<stdin>", &preg);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			enprintf(Error, "fopen %s:", argv[optind]);
		grep(fp, argv[optind], &preg);
		fclose(fp);
	}
	return match ? Match : NoMatch;
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
			exit(Match);
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
	if(ferror(fp))
		enprintf(Error, "%s: read error:", str);
	free(buf);
}
