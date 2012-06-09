/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static int digitsleft(const char *);
static int digitsright(const char *);
static double estrtod(const char *);
static bool validfmt(const char *);

int
main(int argc, char *argv[])
{
	const char *starts = "1", *steps = "1", *ends = "1", *sep = "\n";
	bool wflag = false;
	char c, ftmp[BUFSIZ], *fmt = ftmp;
	double start, step, end, out, dir;

	while((c = getopt(argc, argv, "f:s:w")) != -1)
		switch(c) {
		case 'f':
			if(!validfmt(optarg))
				eprintf("%s: invalid format\n", optarg);
			fmt = optarg;
			break;
		case 's':
			sep = optarg;
			break;
		case 'w':
			wflag = true;
			break;
		}

	switch(argc-optind) {
	case 3:
		starts = argv[optind++];
		steps = argv[optind++];
		ends = argv[optind++];
		break;
	case 2:
		starts = argv[optind++];
		/* fallthrough */
	case 1:
		ends = argv[optind++];
		break;
	default:
		eprintf("usage: %s [-w] [-f fmt] [-s separator] [start [step]] end\n", argv[0]);
	}
	start = estrtod(starts);
	step  = estrtod(steps);
	end   = estrtod(ends);

	dir = (step > 0) ? 1.0 : -1.0;
	if(step == 0 || start * dir > end * dir)
		return EXIT_FAILURE;

	if(fmt == ftmp) {
		int right = MAX(digitsright(starts),
		            MAX(digitsright(ends),
		                digitsright(steps)));

		if(wflag) {
			int left = MAX(digitsleft(starts), digitsleft(ends));

			snprintf(ftmp, sizeof ftmp, "%%0%d.%df", right+left+(right != 0), right);
		}
		else
			snprintf(ftmp, sizeof ftmp, "%%.%df", right);
	}
	for(out = start; out * dir <= end * dir; out += step) {
		if(out != start)
			fputs(sep, stdout);
		printf(fmt, out);
	}
	printf("\n");

	return EXIT_SUCCESS;
}

int
digitsleft(const char *d)
{
	char *exp;
	int shift;

	if(*d == '+')
		d++;
	exp = strpbrk(d, "eE");
	shift = exp ? atoi(&exp[1]) : 0;

	return MAX(0, strspn(d, "-0123456789")+shift);
}

int
digitsright(const char *d)
{
	char *exp;
	int shift, after;

	exp = strpbrk(d, "eE");
	shift = exp ? atoi(&exp[1]) : 0;
	after = (d = strchr(d, '.')) ? strspn(&d[1], "0123456789") : 0;

	return MAX(0, after-shift);
}

double
estrtod(const char *s)
{
	char *end;
	double d;

	d = strtod(s, &end);
	if(end == s || *end != '\0')
		eprintf("%s: not a real number\n", s);
	return d;
}

bool
validfmt(const char *fmt)
{
	int occur = 0;

literal:
	while(*fmt)
		if(*fmt++ == '%')
			goto format;
	return occur == 1;

format:
	if(*fmt == '%') {
		fmt++;
		goto literal;
	}
	fmt += strspn(fmt, "-+#0 '");
	fmt += strspn(fmt, "0123456789");
	if(*fmt == '.') {
		fmt++;
		fmt += strspn(fmt, "0123456789");
	}
	if(*fmt == 'L')
		fmt++;

	switch(*fmt) {
	case 'f': case 'F':
	case 'g': case 'G':
	case 'e': case 'E':
	case 'a': case 'A':
		occur++;
		goto literal;
	default:
		return false;
	}
}
