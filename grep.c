/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

enum { Match = 0, NoMatch = 1, Error = 2 };

static void grep(FILE *, const char *, regex_t *);
static void usage(void);

static bool vflag = false;
static bool many;
static bool match = false;
static char mode = 0;

int
main(int argc, char *argv[])
{
	int i, n, flags = REG_NOSUB;
	regex_t preg;
	FILE *fp;

	ARGBEGIN {
	case 'E':
		flags |= REG_EXTENDED;
		break;
	case 'c':
	case 'l':
	case 'n':
	case 'q':
		mode = ARGC();
		break;
	case 'i':
		flags |= REG_ICASE;
		break;
	case 'v':
		vflag = true;
		break;
	default:
		usage();
	} ARGEND;

	if(argc == 0)
		usage(); /* no pattern */

	if((n = regcomp(&preg, argv[0], flags)) != 0) {
		char buf[BUFSIZ];

		regerror(n, &preg, buf, sizeof buf);
		enprintf(Error, "invalid pattern: %s\n", buf);
	}
	many = (argc > 1);
	if(argc == 1)
		grep(stdin, "<stdin>", &preg);
	else for(i = 1; i < argc; i++) {
		if(!(fp = fopen(argv[i], "r")))
			enprintf(Error, "fopen %s:", argv[i]);
		grep(fp, argv[i], &preg);
		fclose(fp);
	}
	return match ? Match : NoMatch;
}

void
grep(FILE *fp, const char *str, regex_t *preg)
{
	char *buf = NULL;
	long n, c = 0;
	size_t size = 0, len;

	for(n = 1; afgets(&buf, &size, fp); n++) {
		if(buf[(len = strlen(buf))-1] == '\n')
			buf[--len] = '\0';
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
			printf("%s\n", buf);
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

void
usage(void)
{
	enprintf(Error, "usage: %s [-Ecilnqv] pattern [files...]\n", argv0);
}
