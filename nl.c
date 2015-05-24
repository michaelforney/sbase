/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"
#include "utf.h"
#include "util.h"

static size_t   startnum = 1;
static size_t   incr = 1;
static size_t   blines = 1;
static size_t   delimlen = 2;
static int      width = 6;
static int      pflag = 0;
static char     type[] = { 'n', 't', 'n' }; /* footer, body, header */
static char    *delim = "\\:";
static char     format[8] = "%*ld%s";
static char    *sep = "\t";
static regex_t  preg[3];

static int
getsection(char *buf, int *section)
{
	int sectionchanged = 0, newsection = *section;

	for (; !strncmp(buf, delim, delimlen); buf += delimlen) {
		if (!sectionchanged) {
			sectionchanged = 1;
			newsection = 0;
		} else {
			newsection = (newsection + 1) % 3;
		}
	}

	if (!buf[0] || buf[0] == '\n')
		*section = newsection;
	else
		sectionchanged = 0;

	return sectionchanged;
}

static void
nl(const char *fname, FILE *fp)
{
	size_t number = startnum, size = 0, bl = 1;
	int donumber, oldsection, section = 1;
	char *buf = NULL;

	while (getline(&buf, &size, fp) > 0) {
		donumber = 0;
		oldsection = section;

		if (getsection(buf, &section)) {
			if ((section >= oldsection) && !pflag)
				number = startnum;
			continue;
		}

		switch (type[section]) {
		case 't':
			if (buf[0] != '\n')
				donumber = 1;
			break;
		case 'p':
			if (!regexec(preg + section, buf, 0, NULL, 0))
				donumber = 1;
			break;
		case 'a':
			if (buf[0] == '\n' && bl < blines) {
				++bl;
			} else {
				donumber = 1;
				bl = 1;
			}
		}

		if (donumber) {
			printf(format, width, number, sep);
			number += incr;
		}
		fputs(buf, stdout);
	}
	free(buf);
	if (ferror(fp))
		eprintf("getline %s:", fname);
}

static void
usage(void)
{
	eprintf("usage: %s [-p] [-b type] [-d delim] [-f type]\n"
	        "       [-h type] [-i num] [-l num] [-n format]\n"
		"       [-s sep] [-v num] [-w num] [file]\n", argv0);
}

static char
getlinetype(char *type, regex_t *preg)
{
	if (type[0] == 'p')
		eregcomp(preg, type + 1, REG_NOSUB);
	else if (!type[0] || !strchr("ant", type[0]))
		usage();

	return type[0];
}

int
main(int argc, char *argv[])
{
	FILE *fp = NULL;
	size_t l, s;
	int ret = 0;
	char *d, *formattype, *formatblit;

	ARGBEGIN {
	case 'd':
		d = EARGF(usage());
		l = utflen(d);

		switch (l) {
		case 0:
			break;
		case 1:
			s = strlen(d);
			delim = emalloc(s + 1 + 1);
			estrlcpy(delim, d, s + 1 + 1);
			estrlcat(delim, ":", s + 1 + 1);
			delimlen = s + 1;
			break;
		default:
			delim = d;
			delimlen = strlen(delim);
			break;
		}
		break;
	case 'f':
		type[0] = getlinetype(EARGF(usage()), preg);
		break;
	case 'b':
		type[1] = getlinetype(EARGF(usage()), preg + 1);
		break;
	case 'h':
		type[2] = getlinetype(EARGF(usage()), preg + 2);
		break;
	case 'i':
		incr = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
		break;
	case 'l':
		blines = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
		break;
	case 'n':
		formattype = EARGF(usage());
		estrlcpy(format, "%", sizeof(format));

		if (!strcmp(formattype, "ln")) {
			formatblit = "-";
		} else if (!strcmp(formattype, "rn")) {
			formatblit = "";
		} else if (!strcmp(formattype, "rz")) {
			formatblit = "0";
		} else {
			eprintf("%s: bad format\n", formattype);
		}

		estrlcat(format, formatblit, sizeof(format));
		estrlcat(format, "*ld%s", sizeof(format));
		break;
	case 'p':
		pflag = 1;
		break;
	case 's':
		sep = EARGF(usage());
		break;
	case 'v':
		startnum = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
		break;
	case 'w':
		width = estrtonum(EARGF(usage()), 1, INT_MAX);
		break;
	default:
		usage();
	} ARGEND;

	if (argc > 1)
		usage();

	if (!argc) {
		nl("<stdin>", stdin);
	} else {
		if (!strcmp(argv[0], "-")) {
			argv[0] = "<stdin>";
			fp = stdin;
		} else if (!(fp = fopen(argv[0], "r"))) {
			eprintf("fopen %s:", argv[0]);
		}
		nl(argv[0], fp);
	}

	ret |= fp && fp != stdin && fshut(fp, argv[0]);
	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
