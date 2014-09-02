/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "util.h"

static int itostr(char *, int, int);
static FILE *nextfile(FILE *, char *, int, int);

static void
usage(void)
{
	eprintf("usage: split [-d] [-a len] [-b [bytes[k|m|g]]] [-l [lines]] [input [prefix]]\n");
}

static int base = 26, start = 'a';

int
main(int argc, char *argv[])
{
	int plen, slen = 2;
	int ch;
	char name[NAME_MAX+1];
	char *prefix = "x";
	char *file = NULL;
	char *tmp, *end;
	uint64_t size = 1000, scale = 1, n;
	int always = 0;
	FILE *in = stdin, *out = NULL;

	ARGBEGIN {
	case 'b':
		always = 1;
		tmp = ARGF();
		if(tmp == NULL)
			break;

		size = strtoull(tmp, &end, 10);
		if(*end == '\0')
			break;
		switch(toupper((int)*end)) {
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
		if(size > (UINT64_MAX/scale))
			eprintf("'%s': out of range\n", tmp);
		size *= scale;
		break;
	case 'l':
		always = 0;
		tmp = ARGF();
		if(tmp)
			size = estrtol(tmp, 10);
		break;
	case 'a':
		slen = estrtol(EARGF(usage()), 10);
		break;
	case 'd':
		base = 10;
		start = '0';
		break;
	default:
		usage();
	} ARGEND;

	if(*argv)
		file = *argv++;
	if(*argv)
		prefix = *argv++;
	if(*argv)
		usage();

	plen = strlen(prefix);
	if(plen+slen > NAME_MAX)
		eprintf("names cannot exceed %d bytes\n", NAME_MAX);
	strlcpy(name, prefix, sizeof(name));

	if(file && strcmp(file, "-") != 0) {
		in = fopen(file, "r");
		if(!in)
			eprintf("'%s':", file);
	}

Nextfile:
	while((out = nextfile(out, name, plen, slen))) {
		n = 0;
		while((ch = getc(in)) != EOF) {
			putc(ch, out);
			n += (always || ch == '\n');
			if(n >= size)
				goto Nextfile;
		}
		fclose(out);
		break;
	}
	return EXIT_SUCCESS;
}

int
itostr(char *str, int x, int n)
{
	str[n] = '\0';
	while(n-- > 0) {
		str[n] = start + (x % base);
		x /= base;
	}
	if(x)
		return -1;
	return 0;
}

FILE *
nextfile(FILE *f, char *buf, int plen, int slen)
{
	static int n = 0;
	int s;

	if(f)
		fclose(f);
	s = itostr(buf+plen, n++, slen);
	if(s == -1)
		return NULL;

	f = fopen(buf, "w");
	if(!f)
		eprintf("'%s':", buf);
	return f;
}
