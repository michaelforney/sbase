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

static void addpattern(const char *);
static bool grep(FILE *, const char *);

static bool eflag = false;
static bool vflag = false;
static bool many;
static char mode = 0;

static struct plist {
	char *pattern;
	regex_t preg;
	struct plist *next;
} *phead;

static void
usage(void)
{
	enprintf(Error, "usage: %s [-Ecilnqv] [-e pattern] pattern [files...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	bool match = false;
	struct plist *pnode, *tmp;
	int i, n, flags = REG_NOSUB;
	FILE *fp;

	ARGBEGIN {
	case 'E':
		flags |= REG_EXTENDED;
		break;
	case 'c':
	case 'e':
		addpattern(EARGF(usage()));
		eflag = true;
		break;
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

	if(argc == 0 && !eflag)
		usage(); /* no pattern */

	/* If -e is not specified treat it as if it were */
	if(!eflag) {
		addpattern(argv[0]);
		argc--;
		argv++;
	}

	/* Compile regex for all search patterns */
	for(pnode = phead; pnode; pnode = pnode->next) {
		if((n = regcomp(&pnode->preg, pnode->pattern, flags)) != 0) {
			char buf[BUFSIZ];

			regerror(n, &pnode->preg, buf, sizeof buf);
			enprintf(Error, "invalid pattern: %s\n", buf);
		}
	}
	many = (argc > 1);
	if(argc == 0) {
		match = grep(stdin, "<stdin>");
	} else {
		for(i = 0; i < argc; i++) {
			if(!(fp = fopen(argv[i], "r")))
				enprintf(Error, "fopen %s:", argv[i]);
			if(grep(fp, argv[i]))
				match = true;
			fclose(fp);
		}
	}
	pnode = phead;
	while(pnode) {
		tmp = pnode->next;
		regfree(&pnode->preg);
		free(pnode->pattern);
		free(pnode);
		pnode = tmp;
	}
	return match ? Match : NoMatch;
}

void
addpattern(const char *pattern)
{
	struct plist *pnode;

	pnode = malloc(sizeof(*pnode));
	if(!pnode)
		eprintf("malloc:");
	pnode->pattern = strdup(pattern);
	if(!pnode->pattern)
		eprintf("strdup:");
	pnode->next = phead;
	phead = pnode;
}

bool
grep(FILE *fp, const char *str)
{
	char *buf = NULL;
	long n, c = 0;
	size_t size = 0, len;
	struct plist *pnode;
	bool match = false;

	for(n = 1; afgets(&buf, &size, fp); n++) {
		for(pnode = phead; pnode; pnode = pnode->next) {
			if(buf[(len = strlen(buf))-1] == '\n')
				buf[--len] = '\0';
			if(regexec(&pnode->preg, buf, 0, NULL, 0) ^ vflag)
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
	}
	if(mode == 'c')
		printf("%ld\n", c);
end:
	if(ferror(fp))
		enprintf(Error, "%s: read error:", str);
	free(buf);
	return match;
}
