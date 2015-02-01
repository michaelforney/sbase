/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static int base = 26, start = 'a';

int
itostr(char *str, int x, int n)
{
	str[n] = '\0';
	while (n-- > 0) {
		str[n] = start + (x % base);
		x /= base;
	}
	if (x)
		return -1;
	return 0;
}

FILE *
nextfile(FILE *f, char *buf, int plen, int slen)
{
	static int filecount = 0;

	if (f)
		fclose(f);
	if (itostr(buf + plen, filecount++, slen) < 0)
		return NULL;

	if (!(f = fopen(buf, "w")))
		eprintf("'%s':", buf);
	return f;
}

static void
usage(void)
{
	eprintf("usage: %s [-a len] [-b bytes[k|m|g]] [-d] [-l lines] "
	        "[input [prefix]]\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *in = stdin, *out = NULL;
	char name[NAME_MAX + 1];
	char *prefix = "x";
	char *file = NULL;
	char *tmp, *end;
	size_t size = 1000, scale = 1, n;
	int ch, plen, slen = 2, always = 0;
	long l;

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
	strlcpy(name, prefix, sizeof(name));

	if (file && strcmp(file, "-") != 0) {
		if (!(in = fopen(file, "r")))
			eprintf("'%s':", file);
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
	if (in != stdin)
		fclose(in);
	if (out)
		fclose(out);
	return 0;
}
