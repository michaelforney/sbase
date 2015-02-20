/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"
#include "util.h"

#define FORMAT_LN "%-*ld%s%s"
#define FORMAT_RN "%*ld%s%s"
#define FORMAT_RZ "%0*ld%s%s"

static char        mode = 't';
static const char *format = FORMAT_RN;
static const char *sep = "\t";
static int         width = 6;
static size_t      startnum = 1;
static size_t      incr = 1;
static regex_t     preg;

void
nl(const char *name, FILE *fp)
{
	char *buf = NULL;
	size_t size = 0;

	while (getline(&buf, &size, fp) != -1) {
		if ((mode == 'a')
		    || (mode == 'p' && !regexec(&preg, buf, 0, NULL, 0))
		    || (mode == 't' && buf[0] != '\n')) {
			printf(format, width, startnum, sep, buf);
			startnum += incr;
		} else {
			printf("       %s", buf);
		}
	}
	free(buf);
	if (ferror(fp))
		eprintf("%s: read error:", name);
}

static void
usage(void)
{
	eprintf("usage: %s [-b type] [-i incr] [-n format] [-s sep] [-v startnum] [-w width] [file]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *r;

	ARGBEGIN {
	case 'b':
		r = EARGF(usage());
		mode = r[0];
		if (r[0] == 'p')
			eregcomp(&preg, &r[1], REG_NOSUB);
		else if (!strchr("ant", mode))
			usage();
		break;
	case 'i':
		incr = estrtonum(EARGF(usage()), 0, MIN(LLONG_MAX, SIZE_MAX));
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
