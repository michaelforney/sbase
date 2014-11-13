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
static int grep(FILE *, const char *);

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
	struct plist *pnode, *tmp;
	int i, n, m, flags = REG_NOSUB, match = NoMatch;
	char buf[BUFSIZ];
	FILE *fp;

	ARGBEGIN {
	case 'E':
		flags |= REG_EXTENDED;
		break;
	case 'e':
		addpattern(EARGF(usage()));
		eflag = true;
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

	if (argc == 0 && !eflag)
		usage(); /* no pattern */

	/* If -e is not specified treat it as if it were */
	if (!eflag) {
		addpattern(argv[0]);
		argc--;
		argv++;
	}

	/* Compile regex for all search patterns */
	for (pnode = phead; pnode; pnode = pnode->next) {
		if ((n = regcomp(&pnode->preg, pnode->pattern, flags)) != 0) {
			regerror(n, &pnode->preg, buf, sizeof buf);
			enprintf(Error, "invalid pattern: %s\n", buf);
		}
	}
	many = (argc > 1);
	if (argc == 0) {
		match = grep(stdin, "<stdin>");
	} else {
		for (i = 0; i < argc; i++) {
			if (!(fp = fopen(argv[i], "r"))) {
				weprintf("fopen %s:", argv[i]);
				match = Error;
				continue;
			}
			m = grep(fp, argv[i]);
			if (m == Error || (match != Error && m == Match))
				match = m;
			fclose(fp);
		}
	}
	pnode = phead;
	while (pnode) {
		tmp = pnode->next;
		regfree(&pnode->preg);
		free(pnode->pattern);
		free(pnode);
		pnode = tmp;
	}
	return match;
}

static void
addpattern(const char *pattern)
{
	struct plist *pnode;

	if (!(pnode = malloc(sizeof(*pnode))))
		eprintf("malloc:");
	if (!(pnode->pattern = strdup(pattern)))
		eprintf("strdup:");
	pnode->next = phead;
	phead = pnode;
}

static int
grep(FILE *fp, const char *str)
{
	char *buf = NULL;
	size_t len = 0, size = 0;
	long c = 0, n;
	struct plist *pnode;
	int match = NoMatch;

	for (n = 1; (len = agetline(&buf, &size, fp)) != -1; n++) {
		/* Remove the trailing newline if one is present. */
		if (len && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		for (pnode = phead; pnode; pnode = pnode->next) {
			if (regexec(&pnode->preg, buf, 0, NULL, 0) ^ vflag)
				continue;
			switch (mode) {
			case 'c':
				c++;
				break;
			case 'l':
				puts(str);
				goto end;
			case 'q':
				exit(Match);
			default:
				if (many)
					printf("%s:", str);
				if (mode == 'n')
					printf("%ld:", n);
				puts(buf);
				break;
			}
			match = Match;
		}
	}
	if (mode == 'c')
		printf("%ld\n", c);
end:
	if (ferror(fp)) {
		weprintf("%s: read error:", str);
		match = Error;
	}
	free(buf);
	return match;
}
