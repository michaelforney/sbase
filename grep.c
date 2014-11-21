/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "queue.h"
#include "text.h"
#include "util.h"

enum { Match = 0, NoMatch = 1, Error = 2 };

static void addpattern(const char *);
static void addpatternfile(FILE *);
static int grep(FILE *, const char *);

static int Fflag;
static int Hflag;
static int eflag;
static int fflag;
static int hflag;
static int iflag;
static int sflag;
static int vflag;
static int xflag;
static int many;
static int mode;

struct pattern {
	char *pattern;
	regex_t preg;
	SLIST_ENTRY(pattern) entry;
};

static SLIST_HEAD(phead, pattern) phead;

static void
usage(void)
{
	enprintf(Error, "usage: %s [-EFHcilnqsvx] [-e pattern] [-f file] pattern [files...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct pattern *pnode;
	int i, m, flags = REG_NOSUB, match = NoMatch;
	FILE *fp;
	char *arg;

	SLIST_INIT(&phead);

	ARGBEGIN {
	case 'E':
		flags |= REG_EXTENDED;
		break;
	case 'F':
		Fflag = 1;
		break;
	case 'H':
		Hflag = 1;
		break;
	case 'e':
		arg = EARGF(usage());
		fp = fmemopen(arg, strlen(arg) + 1, "r");
		addpatternfile(fp);
		fclose(fp);
		eflag = 1;
		break;
	case 'f':
		arg = EARGF(usage());
		fp = fopen(arg, "r");
		if (!fp)
			enprintf(Error, "fopen %s:", arg);
		addpatternfile(fp);
		fclose(fp);
		fflag = 1;
		break;
	case 'h':
		hflag = 1;
		break;
	case 'c':
	case 'l':
	case 'n':
	case 'q':
		mode = ARGC();
		break;
	case 'i':
		flags |= REG_ICASE;
		iflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'v':
		vflag = 1;
		break;
	case 'x':
		xflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0 && !eflag && !fflag)
		usage(); /* no pattern */

	/* just add literal pattern to list */
	if (!eflag && !fflag) {
		fp = fmemopen(argv[0], strlen(argv[0]) + 1, "r");
		addpatternfile(fp);
		fclose(fp);
		argc--;
		argv++;
	}

	if (!Fflag)
		/* Compile regex for all search patterns */
		SLIST_FOREACH(pnode, &phead, entry)
			enregcomp(Error, &pnode->preg, pnode->pattern, flags);
	many = (argc > 1);
	if (argc == 0) {
		match = grep(stdin, "<stdin>");
	} else {
		for (i = 0; i < argc; i++) {
			if (!(fp = fopen(argv[i], "r"))) {
				if (!sflag)
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
	while (!SLIST_EMPTY(&phead)) {
		pnode = SLIST_FIRST(&phead);
		SLIST_REMOVE_HEAD(&phead, entry);
		if (!Fflag)
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
	char *tmp;

	/* a null BRE/ERE matches every line */
	if (!Fflag)
		if (pattern[0] == '\0')
			pattern = ".";

	if (!Fflag && xflag) {
		tmp = emalloc(strlen(pattern) + 3);
		snprintf(tmp, strlen(pattern) + 3, "%s%s%s",
			 pattern[0] == '^' ? "" : "^",
			 pattern,
			 pattern[strlen(pattern) - 1] == '$' ? "" : "$");
	} else {
		tmp = estrdup(pattern);
	}

	pnode = emalloc(sizeof(*pnode));
	pnode->pattern = tmp;
	SLIST_INSERT_HEAD(&phead, pnode, entry);
}

static void
addpatternfile(FILE *fp)
{
	char *buf = NULL;
	size_t len = 0, size = 0;

	while ((len = getline(&buf, &size, fp)) != -1) {
		if (len && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		addpattern(buf);
	}
	free(buf);
}

static int
grep(FILE *fp, const char *str)
{
	char *buf = NULL;
	size_t len = 0, size = 0;
	long c = 0, n;
	struct pattern *pnode;
	int match = NoMatch;

	for (n = 1; (len = getline(&buf, &size, fp)) != -1; n++) {
		/* Remove the trailing newline if one is present. */
		if (len && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		SLIST_FOREACH(pnode, &phead, entry) {
			if (!Fflag) {
				if (regexec(&pnode->preg, buf[0] == '\0' ? "\n" : buf, 0, NULL, 0) ^ vflag)
					continue;
			} else {
				if (!xflag) {
					if ((iflag ? strcasestr : strstr)(buf, pnode->pattern))
						match = Match;
					else
						match = NoMatch;
				} else {
					if (!(iflag ? strcasecmp : strcmp)(buf, pnode->pattern))
						match = Match;
					else
						match = NoMatch;
				}
				if (match ^ vflag)
					continue;
			}
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
				if (!hflag && (many || Hflag))
					printf("%s:", str);
				if (mode == 'n')
					printf("%ld:", n);
				puts(buf);
				break;
			}
			match = Match;
			break;
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
