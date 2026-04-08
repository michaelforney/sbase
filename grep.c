/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "queue.h"
#include "util.h"

enum { Match = 0, NoMatch = 1, Error = 2 };

static void addpattern(const char *);
static void addpatternfile(FILE *);
static int grep(FILE *, const char *);

static int Eflag;
static int Fflag;
static int Hflag;
static int eflag;
static int fflag;
static int hflag;
static int iflag;
static int sflag;
static int vflag;
static int wflag;
static int xflag;
static int many;
static int mode;

struct pattern {
	regex_t preg;
	SLIST_ENTRY(pattern) entry;
	char pattern[];
};

static SLIST_HEAD(phead, pattern) phead;

static void
addpattern(const char *pattern)
{
	struct pattern *pnode;
	size_t patlen;

	patlen = strlen(pattern);

	pnode = enmalloc(Error, sizeof(*pnode) + patlen + 9);
	SLIST_INSERT_HEAD(&phead, pnode, entry);

	if (Fflag || (!xflag && !wflag)) {
		memcpy(pnode->pattern, pattern, patlen + 1);
	} else {
		sprintf(pnode->pattern, "%s%s%s%s%s",
			xflag ? "^" : "\\<",
			Eflag ? "(" : "\\(",
			pattern,
			Eflag ? ")" : "\\)",
			xflag ? "$" : "\\>");
	}
}

static void
addpatternfile(FILE *fp)
{
	static char *buf = NULL;
	static size_t size = 0;
	ssize_t len = 0;

	while ((len = getline(&buf, &size, fp)) > 0) {
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		addpattern(buf);
	}
	if (ferror(fp))
		enprintf(Error, "read error:");
}

static int
grep(FILE *fp, const char *str)
{
	static char *buf = NULL;
	static size_t size = 0;
	ssize_t len = 0;
	long c = 0, n;
	struct pattern *pnode;
	int match, result = NoMatch;

	for (n = 1; (len = getline(&buf, &size, fp)) > 0; n++) {
		/* Remove the trailing newline if one is present. */
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		match = 0;
		SLIST_FOREACH(pnode, &phead, entry) {
			if (Fflag) {
				if (xflag) {
					if (!(iflag ? strcasecmp : strcmp)(buf, pnode->pattern)) {
						match = 1;
						break;
					}
				} else {
					if ((iflag ? strcasestr : strstr)(buf, pnode->pattern)) {
						match = 1;
						break;
					}
				}
			} else {
				if (regexec(&pnode->preg, buf, 0, NULL, 0) == 0) {
					match = 1;
					break;
				}
			}
		}
		if (match != vflag) {
			result = Match;
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
		}
	}
	if (mode == 'c')
		printf("%ld\n", c);
end:
	if (ferror(fp)) {
		weprintf("%s: read error:", str);
		result = Error;
	}
	return result;
}

static void
usage(void)
{
	enprintf(Error, "usage: %s [-EFHchilnqsvwx] [-e pattern] [-f file] "
	         "[pattern] [file ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct pattern *pnode;
	int m, flags = REG_NOSUB, match = NoMatch;
	FILE *fp;
	char *arg;

	SLIST_INIT(&phead);

	ARGBEGIN {
	case 'E':
		Eflag = 1;
		Fflag = 0;
		flags |= REG_EXTENDED;
		break;
	case 'F':
		Fflag = 1;
		Eflag = 0;
		flags &= ~REG_EXTENDED;
		break;
	case 'H':
		Hflag = 1;
		hflag = 0;
		break;
	case 'e':
		arg = EARGF(usage());
		if (!(fp = fmemopen(arg, strlen(arg) + 1, "r")))
			eprintf("fmemopen:");
		addpatternfile(fp);
		efshut(fp, arg);
		eflag = 1;
		break;
	case 'f':
		arg = EARGF(usage());
		fp = fopen(arg, "r");
		if (!fp)
			enprintf(Error, "fopen %s:", arg);
		addpatternfile(fp);
		efshut(fp, arg);
		fflag = 1;
		break;
	case 'h':
		hflag = 1;
		Hflag = 0;
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
	case 'w':
		wflag = 1;
		break;
	case 'x':
		xflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (argc == 0 && !eflag && !fflag)
		usage(); /* no pattern */

	/* just add literal pattern to list */
	if (!eflag && !fflag) {
		if (!(fp = fmemopen(argv[0], strlen(argv[0]) + 1, "r")))
			eprintf("fmemopen:");
		addpatternfile(fp);
		efshut(fp, argv[0]);
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
		for (; *argv; argc--, argv++) {
			if (!strcmp(*argv, "-")) {
				*argv = "<stdin>";
				fp = stdin;
			} else if (!(fp = fopen(*argv, "r"))) {
				if (!sflag)
					weprintf("fopen %s:", *argv);
				match = Error;
				continue;
			}
			m = grep(fp, *argv);
			if (m == Error || (match != Error && m == Match))
				match = m;
			if (fp != stdin && fshut(fp, *argv))
				match = Error;
		}
	}

	if (fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>"))
		match = Error;

	return match;
}
