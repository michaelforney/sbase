/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"
#include "text.h"
#include "util.h"

enum { Match = 0, NoMatch = 1, Error = 2 };

static void addpattern(const char *);
static int grep(FILE *, const char *);

static int eflag = 0;
static int vflag = 0;
static int Hflag = 0;
static int many;
static char mode = 0;

struct pattern {
	char *pattern;
	regex_t preg;
	TAILQ_ENTRY(pattern) entry;
};

static TAILQ_HEAD(phead, pattern) phead;

static void
usage(void)
{
	enprintf(Error, "usage: %s [-EHcilnqv] [-e pattern] pattern [files...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct pattern *pnode, *tmp;
	int i, m, flags = REG_NOSUB, match = NoMatch;
	FILE *fp;

	TAILQ_INIT(&phead);

	ARGBEGIN {
	case 'E':
		flags |= REG_EXTENDED;
		break;
	case 'H':
		Hflag = 1;
		break;
	case 'e':
		addpattern(EARGF(usage()));
		eflag = 1;
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
		vflag = 1;
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
	TAILQ_FOREACH(pnode, &phead, entry) {
		enregcomp(Error, &pnode->preg, pnode->pattern, flags);
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
	TAILQ_FOREACH_SAFE(pnode, &phead, entry, tmp) {
		TAILQ_REMOVE(&phead, pnode, entry);
		regfree(&pnode->preg);
		free(pnode->pattern);
		free(pnode);
	}
	return match;
}

static void
addpattern(const char *pattern)
{
	struct pattern *pnode;

	pnode = emalloc(sizeof(*pnode));
	pnode->pattern = estrdup(pattern);
	TAILQ_INSERT_TAIL(&phead, pnode, entry);
}

static int
grep(FILE *fp, const char *str)
{
	char *buf = NULL;
	size_t len = 0, size = 0;
	long c = 0, n;
	struct pattern *pnode;
	int match = NoMatch;

	for (n = 1; (len = agetline(&buf, &size, fp)) != -1; n++) {
		/* Remove the trailing newline if one is present. */
		if (len && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		TAILQ_FOREACH(pnode, &phead, entry) {
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
				if (many || Hflag)
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
