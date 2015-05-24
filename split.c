/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static int base = 26, start = 'a';

static int
itostr(char *str, int x, int n)
{
	str[n] = '\0';
	while (n-- > 0) {
		str[n] = start + (x % base);
		x /= base;
	}

	return x ? -1 : 0;
}

static FILE *
nextfile(FILE *f, char *buf, int plen, int slen)
{
	static int filecount = 0;

	if (f)
		fshut(f, "<file>");
	if (itostr(buf + plen, filecount++, slen) < 0)
		return NULL;

	if (!(f = fopen(buf, "w")))
		eprintf("'%s':", buf);

	return f;
}

static void
usage(void)
{
	eprintf("usage: %s [-a num] [-b num[k|m|g] | -l num] [-d] "
	        "[file [prefix]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *in = stdin, *out = NULL;
	size_t size = 1000, scale = 1, n;
	long l;
	int ret = 0, ch, plen, slen = 2, always = 0;
	char name[NAME_MAX + 1], *prefix = "x", *file = NULL, *tmp, *end;

	ARGBEGIN {
	case 'a':
		slen = estrtonum(EARGF(usage()), 0, INT_MAX);
		break;
	case 'b':
		always = 1;
		tmp = EARGF(usage());
		l = strtol(tmp, &end, 10);
		if (l <= 0)
			eprintf("invalid number of bytes: %s\n", tmp);
		size = (size_t)l;
		if (!*end)
			break;
		switch (toupper((int)*end)) {
		case 'K':
			scale = 1024;
			break;
		case 'M':
			scale = 1024L * 1024L;
			break;
		case 'G':
			scale = 1024L * 1024L * 1024L;
			break;
		default:
			usage();
		}
		if (size > (SIZE_MAX / scale))
			eprintf("'%s': out of range\n", tmp);
		size *= scale;
		break;
	case 'd':
		base = 10;
		start = '0';
		break;
	case 'l':
		always = 0;
		tmp = EARGF(usage());
		size = estrtonum(tmp, 1, MIN(LLONG_MAX, SIZE_MAX));
		break;
	default:
		usage();
	} ARGEND;

	if (*argv)
		file = *argv++;
	if (*argv)
		prefix = *argv++;
	if (*argv)
		usage();

	plen = strlen(prefix);
	if (plen + slen > NAME_MAX)
		eprintf("names cannot exceed %d bytes\n", NAME_MAX);
	estrlcpy(name, prefix, sizeof(name));

	if (file && strcmp(file, "-")) {
		if (!(in = fopen(file, "r")))
			eprintf("fopen %s:", file);
	}

	n = 0;
	while ((ch = getc(in)) != EOF) {
		if (!out || n >= size) {
			if (!(out = nextfile(out, name, plen, slen)))
				eprintf("fopen: %s:", name);
			n = 0;
		}
		n += (always || ch == '\n');
		putc(ch, out);
	}

	ret |= (in != stdin) && fshut(in, "<infile>");
	ret |= out && (out != stdout) && fshut(out, "<outfile>");
	ret |= fshut(stdin, "<stdin>") | fshut(stdout, "<stdout>");

	return ret;
}
