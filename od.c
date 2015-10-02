/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static size_t bytes_per_line = 16;
static off_t maxbytes = -1;
static off_t skip = 0;
static unsigned char radix = 'o';
static unsigned char type = 'o';

static void
printaddress(FILE *f, off_t addr)
{
	char fmt[] = "%07j# ";

	if (radix == 'n') {
		fputc(' ', f);
	} else {
		fmt[4] = radix;
		fprintf(f, fmt, (intmax_t)addr);
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

	switch (type) {
	case 'a':
		c &= ~128; /* clear high bit as required by standard */
		if (c < LEN(namedict) || c == 127) {
			fprintf(f, "%3s ", (c == 127) ? "del" : namedict[c]);
		} else {
			fprintf(f, "%3c ", c);
		}
		break;
	case 'c':
		if (strchr("\a\b\t\n\v\f\r\0", c)) {
			fprintf(f, "%3s ", escdict[c]);
		} else {
			fprintf(f, "%3c ", c);
		}
		break;
	default:
		fprintf(f, fmtdict[type], c);
	}
}

static void
od(FILE *in, char *in_name, FILE *out, char *out_name)
{
	off_t addr;
	size_t i, chunklen;
	unsigned char buf[BUFSIZ];

	for (addr = 0; (chunklen = fread(buf, 1, BUFSIZ, in)); ) {
		for (i = 0; i < chunklen && (maxbytes == -1 ||
		     (addr - skip) < maxbytes); ++i, ++addr) {
			if (addr - skip < 0)
				continue;
			if (((addr - skip) % bytes_per_line) == 0) {
				if (addr - skip)
					fputc('\n', out);
				printaddress(out, addr);
			}
			printchar(out, buf[i]);
		}
		if (feof(in) || ferror(in) || ferror(out))
			break;
	}
	if (addr - skip > 0)
		fputc('\n', out);
	if (radix != 'n') {
		printaddress(out, MAX(addr, skip));
		fputc('\n', out);
	}
}

static void
usage(void)
{
	eprintf("usage: %s [-A d|o|x|n] [-t a|c|d|o|u|x] [-v] [file ...]\n", argv0);
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
	case 'j':
		if ((skip = parseoffset(EARGF(usage()))) < 0)
			return 1;
		break;
	case 'N':
		if ((maxbytes = parseoffset(EARGF(usage()))) < 0)
			return 1;
		break;
	case 't':
		s = EARGF(usage());
		if (strlen(s) != 1 || !strchr("acdoux", s[0]))
			usage();
		type = s[0];
		break;
	case 'v':
		/* Always set. Use "uniq -f 1 -c" to handle duplicate lines. */
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
