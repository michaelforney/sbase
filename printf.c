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
	eprintf("usage: %s format [arg ...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	Rune *rarg;
	size_t i, j, argi, lastargi, formatlen, arglen;
	long long num;
	double dou;
	int cooldown = 0, width, precision;
	char *format, *tmp, *arg, *fmt, flag;

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

		/* flag */
		for (flag = '\0', i++; strchr("#-+ 0", format[i]); i++) {
			flag = format[i];
		}

		/* field width */
		width = -1;
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
				width = estrtonum(tmp, 0, INT_MAX);
				free(tmp);
			} else {
				width = 0;
			}
		}

		/* field precision */
		precision = -1;
		if (format[i] == '.') {
			if (format[++i] == '*') {
				if (argi < argc)
					precision = estrtonum(argv[argi++], 0, INT_MAX);
				else
					cooldown = 1;
				i++;
			} else {
				j = i;
				for (; strchr("+-0123456789", format[i]); i++);
				if (j != i) {
					tmp = estrndup(format + j, i - j);
					precision = estrtonum(tmp, 0, INT_MAX);
					free(tmp);
				} else {
					precision = 0;
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
			rarg = ereallocarray(NULL, utflen(arg) + 1, sizeof(*rarg));
			utftorunestr(arg, rarg);
			efputrune(rarg, stdout, "<stdout>");
			free(rarg);
			break;
		case 's':
			printf("%*.*s", width, precision, arg);
			break;
		case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
			arglen = strlen(arg);
			for (j = 0; j < arglen && isspace(arg[j]); j++);
			if (arg[j] == '\'' || arg[j] == '\"') {
				arg += j + 1;
				unescape(arg);
				rarg = ereallocarray(NULL, utflen(arg) + 1, sizeof(*rarg));
				utftorunestr(arg, rarg);
				num = rarg[0];
			} else {
				num = (strlen(arg) > 0) ? estrtonum(arg, LLONG_MIN, LLONG_MAX) : 0;
			}
			fmt = estrdup(flag ? "%#*.*ll#" : "%*.*ll#");
			if (flag)
				fmt[1] = flag;
			fmt[flag ? 7 : 6] = format[i];
			printf(fmt, width, precision, num);
			free(fmt);
			break;
		case 'a': case 'A': case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
			fmt = estrdup(flag ? "%#*.*#" : "%*.*#");
			if (flag)
				fmt[1] = flag;
			fmt[flag ? 5 : 4] = format[i];
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

	return fshut(stdout, "<stdout>");
}
