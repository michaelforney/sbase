/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utf.h"
#include "util.h"

static void
usage(void)
{
	eprintf("%s format [arg ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	Rune *rarg;
	size_t i, j, argi, lastargi, formatlen, arglen;
	long long num;
	double dou;
	int cooldown = 0, width, precision;
	char *format, *tmp, *arg, *fmt;

	argv0 = argv[0];
	if (argc < 2)
		usage();

	format = argv[1];
	if ((tmp = strstr(format, "\\c"))) {
		*tmp = 0;
		cooldown = 1;
	}
	formatlen = unescape(format);
	if (formatlen == 0)
		return 0;
	lastargi = 0;
	for (i = 0, argi = 2; !cooldown || i < formatlen; i++, i = cooldown ? i : (i % formatlen)) {
		if (i == 0) {
			if (lastargi == argi)
				break;
			lastargi = argi;
		}
		if (format[i] != '%') {
			putchar(format[i]);
			continue;
		}

		/* field width */
		width = 0;
		for (i++; strchr("#-+ 0", format[i]); i++);
		if (format[i] == '*') {
			if (argi < argc)
				width = estrtonum(argv[argi++], 0, INT_MAX);
			else
				cooldown = 1;
			i++;
		} else {
			j = i;
			for (; strchr("+-0123456789", format[i]); i++);
			if (j != i) {
				tmp = estrndup(format + j, i - j);
				tmp[i - j] = 0;
				width = estrtonum(tmp, 0, INT_MAX);
				free(tmp);
			}
		}

		/* field precision */
		precision = 6;
		if (format[i] == '.') {
			if (format[++i] == '*') {
				if (argi < argc)
					precision = estrtonum(argv[argi++], 0, INT_MAX);
				else
					cooldown = 1;
			} else {
				j = i;
				for (; strchr("+-0123456789", format[i]); i++);
				if (j != i) {
					tmp = estrndup(format + j, i - j);
					tmp[i - j] = 0;
					precision = estrtonum(tmp, 0, INT_MAX);
					free(tmp);
				}
			}
		}

		if (format[i] != '%') {
			if (argi < argc)
				arg = argv[argi++];
			else {
				arg = "";
				cooldown = 1;
			}
		} else
			putchar('%');

		switch (format[i]) {
		case 'b':
			if ((tmp = strstr(arg, "\\c"))) {
				*tmp = 0;
				unescape(arg);
				fputs(arg, stdout);
				return 0;
			}
			unescape(arg);
			fputs(arg, stdout);
			break;
		case 'c':
			unescape(arg);
			rarg = emalloc((utflen(arg) + 1) * sizeof(*rarg));
			utftorunestr(arg, rarg);
			efputrune(rarg, stdout, "<stdout>");
			free(rarg);
			break;
		case 's':
			fputs(arg, stdout);
			break;
		case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
			arglen = strlen(arg);
			for (j = 0; j < arglen && isspace(arg[j]); j++);
			if (arg[j] == '\'' || arg[j] == '\"') {
				arg += j + 1;
				unescape(arg);
				rarg = emalloc((utflen(arg) + 1) * sizeof(*rarg));
				utftorunestr(arg, rarg);
				num = rarg[0];
			} else
				num = (strlen(arg) > 0) ? estrtonum(arg, LLONG_MIN, LLONG_MAX) : 0;
			fmt = estrdup("%*ll#");
			fmt[4] = format[i];
			printf(fmt, width, num);
			free(fmt);
			break;
		case 'a': case 'A': case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
			fmt = estrdup("%*.*#");
			fmt[4] = format[i];
			dou = (strlen(arg) > 0) ? estrtod(arg) : 0;
			printf(fmt, width, precision, dou);
			free(fmt);
			break;
		default:
			eprintf("Invalid format specifier '%c'.\n", format[i]);
		}
		if (argi >= argc)
			cooldown = 1;
	}
	return 0;
}
