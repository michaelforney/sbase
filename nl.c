/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"
#include "utf.h"
#include "util.h"

/* formats here specify line number and separator (not line content) */
#define FORMAT_LN "%-*ld%s"
#define FORMAT_RN "%*ld%s"
#define FORMAT_RZ "%0*ld%s"

static char        type[] = { 'n', 't', 'n' }; /* footer, body, header */
static char       *delim = "\\:";
static const char *format = FORMAT_RN;
static const char *sep = "\t";
static int         width = 6;
static int         pflag = 0;
static size_t      startnum = 1;
static size_t      incr = 1;
static size_t      blines = 1;
static size_t      delimlen = 2;
static regex_t     preg[3];

static int
getsection(char *buf, int *section)
{
	int sectionchanged = 0;
	int newsection = *section;

	for (; !strncmp(buf, delim, delimlen); buf += delimlen) {
		if (!sectionchanged) {
			sectionchanged = 1;
			newsection = 0;
		} else {
			++newsection;
			newsection %= 3;
		}
	}

	if (buf && buf[0] == '\n')
		*section = newsection;
	else
		sectionchanged = 0;

	return sectionchanged;
}

static void
nl(const char *name, FILE *fp)
{
	char *buf = NULL;
	int donumber, oldsection, section = 1, bl = 1;
	size_t number = startnum, size = 0;

	while (getline(&buf, &size, fp) != -1) {
		donumber = 0;
		oldsection = section;

		if (getsection(buf, &section)) {
			if ((section >= oldsection) && !pflag)
				number = startnum;
			continue;
		}

		if ((type[section] == 't' && buf[0] != '\n')
		    || (type[section] == 'p' &&
		        !regexec(&preg[section], buf, 0, NULL, 0))) {
			donumber = 1;
		} else if (type[section] == 'a') {
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
		} else {
			printf("%*s", width, "");
		}
		printf("%s", buf);
	}
	free(buf);
	if (ferror(fp))
		eprintf("%s: read error:", name);
}

static void
usage(void)
{
	eprintf("usage: %s [-p] [-b type] [-d delim] [-f type] "
	    "[-h type] [-i incr] [-l num]\n[-n format] [-s sep] "
	    "[-v startnum] [-w width] [file]\n", argv0);
}
static char
getlinetype(char *type, regex_t *preg)
{
	if (type[0] == 'p')
		eregcomp(preg, &type[1], REG_NOSUB);
	else if (!strchr("ant", type[0]))
		usage();

	return type[0];
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *d;
	size_t l, s;

	ARGBEGIN {
	case 'b':
		type[1] = getlinetype(EARGF(usage()), &preg[1]);
		break;
	case 'd':
		d = EARGF(usage());
		l = utflen(d);

		switch (l) {
		case 0:
			break;
		case 1:
			s = strlen(d);
			delim = emalloc(s + 2);
			estrlcpy(delim, d, s + 2);
			estrlcat(delim, ":", s + 2);
			delimlen = s + 1;
			break;
		default:
			delim = d;
			delimlen = strlen(delim);
			break;
		}
		break;
	case 'f':
		type[0] = getlinetype(EARGF(usage()), &preg[0]);
		break;
	case 'h':
		type[2] = getlinetype(EARGF(usage()), &preg[2]);
		break;
	case 'i':
		incr = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
		break;
	case 'l':
		blines = estrtonum(EARGF(usage()), 0, UINT_MAX);
		break;
	case 'n':
		format = EARGF(usage());
		if (!strcmp(format, "ln"))
			format = FORMAT_LN;
		else if (!strcmp(format, "rn"))
			format = FORMAT_RN;
		else if (!strcmp(format, "rz"))
			format = FORMAT_RZ;
		else
			eprintf("%s: bad format\n", format);
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

	if (argc == 0) {
		nl("<stdin>", stdin);
	} else {
		if (!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		nl(argv[0], fp);
		fclose(fp);
	}
	return 0;
}
