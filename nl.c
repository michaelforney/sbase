/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static char        mode = 't';
static const char *sep = "\t";
static size_t      incr = 1;
static regex_t     preg;

void
nl(const char *name, FILE *fp)
{
	char *buf = NULL;
	size_t n = 0, size = 0;

	while (getline(&buf, &size, fp) != -1) {
		if ((mode == 'a')
		    || (mode == 'p' && !regexec(&preg, buf, 0, NULL, 0))
		    || (mode == 't' && buf[0] != '\n'))
			printf("%6ld%s%s", n += incr, sep, buf);
		else
			printf("       %s", buf);
	}
	free(buf);
	if (ferror(fp))
		eprintf("%s: read error:", name);
}

static void
usage(void)
{
	eprintf("usage: %s [-b type] [-i incr] [-s sep] [file]\n", argv0);
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
	case 's':
		sep = EARGF(usage());
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
