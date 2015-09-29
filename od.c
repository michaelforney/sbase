/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <string.h>

#include "util.h"

static size_t bytes_per_line = 16;
static unsigned char radix = 'o';
static unsigned char type = 'o';

static void
usage(void)
{
	eprintf("usage: %s [-A d|o|x|n] [-t a|c|d|o|u|x] [file ...]\n", argv0);
}

static void
printaddress(FILE *f, size_t addr)
{
	char fmt[] = "%06z# ";

	if (radix == 'n') {
		fputc(' ', f);
	} else {
		fmt[4] = radix;
		fprintf(f, fmt, addr);
	}
}

static void
printchar(FILE *f, unsigned char c)
{
	const char *namedict[] = {
		"nul", "soh", "stx", "etx", "eot", "enq", "ack",
		"bel", "bs",  "ht",  "nl",  "vt",  "ff",  "cr",
		"so",  "si",  "dle", "dc1", "dc2", "dc3", "dc4",
		"nak", "syn", "etb", "can", "em",  "sub", "esc",
		"fs",  "gs",  "rs",  "us",  "sp",
	};
	const char *escdict[] = {
		['\0'] = "\\0", ['\a'] = "\\a",
		['\b'] = "\\b", ['\t'] = "\\t",
		['\n'] = "\\n", ['\v'] = "\\v",
		['\f'] = "\\f", ['\r'] = "\\r",
	};
	const char *fmtdict[] = {
		['d'] = "%4hhd ", ['o'] = "%03hho ",
		['u'] = "%3hhu ", ['x'] = "%02hhx ",
	};

	if (type != 'a' && type != 'c') {
		fprintf(f, fmtdict[type], c);
	} else {
		switch (type) {
		case 'a':
			c &= ~128; /* clear high bit as required by standard */
			if (c < LEN(namedict) || c == 127) {
				fprintf(f, "%3s ", (c == 127) ? "del" : namedict[c]);
				return;
			}
			break;
		case 'c':
			if (strchr("\a\b\t\n\b\f\r\0", c)) {
				fprintf(f, "%3s ", escdict[c]);
				return;
			}
			break;
		}
		fprintf(f, "%3c ", c);
	}
}

static void
od(FILE *in, char *in_name, FILE *out, char *out_name)
{
	unsigned char buf[BUFSIZ];
	char fmt[] = "\n%.6z#";
	off_t addr, bread, i;

	addr = 0;
	for (; (bread = fread(buf, 1, BUFSIZ, in)); ) {
		for (i = 0; i < bread; ++i, ++addr) {
			if ((addr % bytes_per_line) == 0) {
				if (addr)
					fputc('\n', out);
				printaddress(out, addr);
			}
			printchar(out, buf[i]);
		}
		if (feof(in) || ferror(in) || ferror(out))
			break;
	}
	if (radix != 'n') {
		fmt[5] = radix;
		fprintf(out, fmt, addr);
	}
	fputc('\n', out);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ret = 0;
	char *s;

	ARGBEGIN {
	case 'A':
		s = EARGF(usage());
		if (strlen(s) != 1 || !strchr("doxn", s[0]))
			usage();
		radix = s[0];
		break;
	case 't':
		s = EARGF(usage());
		if (strlen(s) != 1 || !strchr("acdoux", s[0]))
			usage();
		type = s[0];
		break;
	default:
		usage();
	} ARGEND;

	if (!argc) {
		od(stdin, "<stdin>", stdout, "<stdout>");
	} else {
		for (; *argv; argc--, argv++) {
			if (!strcmp(*argv, "-")) {
				*argv = "<stdin>";
				fp = stdin;
			} else if (!(fp = fopen(*argv, "r"))) {
				weprintf("fopen %s:", *argv);
				ret = 1;
				continue;
			}
			od(fp, *argv, stdout, "<stdout>");
			if (fp != stdin && fshut(fp, *argv))
				ret = 1;
		}
	}

	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>") |
	       fshut(stderr, "<stderr>");

	return ret;
}
