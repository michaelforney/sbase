/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>
#include <regex.h>

#include "util.h"

#define MAX(a, b) (((a) > (b))? (a):(b))

int
validfloat(char *fmt)
{
	char *end;

	fmt += strspn(fmt, " ");
	strtod(fmt, &end);
	if (fmt == end || end != fmt + strlen(fmt))
		return 0;

	return 1;
}

int
digitsleft(char *d)
{
	char *exp;
	int shift;

	if (d[0] == '-' || d[0] == '+')
		d++;
	exp = strpbrk(d, "eE");
	shift = exp? atoi(exp+1) : 0;

	return MAX(0, strspn(d, "0123456789")+shift);
}

int
digitsright(char *d)
{
	char *exp;
	int shift, after;

	if (d[0] == '-' || d[0] == '+')
		d++;
	exp = strpbrk(d, "eE");
	shift = exp ? atoi(exp+1) : 0;
	after = (d = strchr(d, '.'))? strspn(d+1, "0123456789") : 0;

	return MAX(0, after-shift);
}

int
validfmt(char *fmt)
{
	regex_t reg;
	int ret;

	regcomp(&reg, "\\([^%]|%%\\)*%[0-9]*\\.[0-9]*[fFgG]"
			"\\([^%]|%%\\)*", REG_NOSUB);
	ret = regexec(&reg, fmt, 0, NULL, 0);
	regfree(&reg);

	return (ret == 0);
}

int
main(int argc, char *argv[])
{
	char c, *fmt, ftmp[4096], *sep, *starts, *steps, *ends;
	bool wflag, fflag;
	double start, step, end, out;
	int left, right;

	sep = "\n";
	fmt = ftmp;

	wflag = false;
	fflag = false;

	starts = "1";
	steps = "1";
	ends = "1";

	while((c = getopt(argc, argv, "f:s:w")) != -1) {
		switch(c) {
		case 'f':
			if (!validfmt(optarg))
				eprintf("invalid format.\n");
			fmt = optarg;
			fflag = true;
			break;
		case 's':
			sep = optarg;
			break;
		case 'w':
			wflag = true;
			break;
		}
	}

	if (wflag && fflag)
		eprintf("-f and -w cannot be combined.\n");

	switch(argc-optind) {
	case 3:
		starts = argv[optind++];
		steps = argv[optind++];
		ends = argv[optind++];
		break;
	case 2:
		starts = argv[optind++];
		ends = argv[optind++];
		break;
	case 1:
		ends = argv[optind++];
		break;
	default:
		eprintf("usage: %s [-w] [-f fmt] [-s separator] "
				"[start [step]] end\n",
				basename(argv[0]));
	}

	if (!validfloat(starts))
		eprintf("start is not a valid float.\n");
	if (!validfloat(steps))
		eprintf("step is not a valid float.\n");
	if (!validfloat(ends))
		eprintf("end is not a valid float.\n");

	start = atof(starts);
	step = atof(steps);
	end = atof(ends);

	if (step == 0)
		return EXIT_FAILURE;

	if (start > end) {
		if (step > 0)
			return EXIT_FAILURE;
	} else if (start < end) {
		if (step < 0)
			return EXIT_FAILURE;
	}

	right = MAX(digitsright(starts),
			MAX(digitsright(ends),
				digitsright(steps)));
	if (wflag) {
		left = MAX(digitsleft(starts), digitsleft(ends));

		snprintf(ftmp, sizeof(ftmp), "%%0%d.%df",
				right+left+(right != 0),
				right);
	} else if (fmt == ftmp) {
		snprintf(ftmp, sizeof(ftmp), "%%.%df", right);
	}

	for (out = start;;) {
		printf(fmt, out);

		out += step;
		if (start > end) {
			if (out >= end) {
				printf("%s", sep);
			} else {
				break;
			}
		} else if (start < end) {
			if (out <= end) {
				printf("%s", sep);
			} else {
				break;
			}
		} else {
			break;
		}
	}
	printf("\n");

	return EXIT_SUCCESS;
}

