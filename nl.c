/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "text.h"
#include "util.h"

static void nl(const char *, FILE *);

static char mode = 't';
static const char *sep = "\t";
static long incr = 1;
static regex_t preg;

static void
usage(void)
{
	eprintf("usage: %s [-b style] [-i increment] [-s sep] [FILE...]\n",
			argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	char *r;
	int ret = 0;

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
		incr = estrtol(EARGF(usage()), 0);
		break;
	case 's':
		sep = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0) {
		nl("<stdin>", stdin);
	} else for (; argc > 0; argc--, argv++) {
		if (!(fp = fopen(argv[0], "r"))) {
			weprintf("fopen %s:", argv[0]);
			ret = 1;
			continue;
		}
		nl(argv[0], fp);
		fclose(fp);
	}
	return ret;
}

void
nl(const char *name, FILE *fp)
{
	char *buf = NULL;
	long n = 0;
	size_t size = 0;

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
