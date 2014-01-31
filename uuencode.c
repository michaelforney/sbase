/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"

static void uuencode(FILE *, const char *, const char *);

static void
usage(void)
{
	eprintf("usage: %s [file] name\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp;

	ARGBEGIN {
	case 'm':
		eprintf("-m not implemented\n");
	default:
		usage();
	} ARGEND;

	if (argc == 0 || argc > 2)
		usage();

	if (argc == 1) {
		uuencode(stdin, argv[0], "<stdin>");
	} else {
		if (!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		uuencode(fp, argv[1], argv[0]);
		fclose(fp);
	}
	return EXIT_SUCCESS;
}

static void
uuencode(FILE *fp, const char *name, const char *s)
{
	struct stat st;
	unsigned char buf[45], *p;
	ssize_t n;
	int ch;

	if (fstat(fileno(fp), &st) < 0)
		eprintf("fstat %s:", s);
	fprintf(stdout, "begin %o %s\n", st.st_mode & 0777, name);
	while ((n = fread(buf, 1, sizeof(buf), fp))) {
		ch = ' ' + (n & 0x3f);
		putchar(ch == ' ' ? '`' : ch);
		for (p = buf; n > 0; n -= 3, p += 3) {
			if (n < 3) {
				p[2] = '\0';
				if (n < 2)
					p[1] = '\0';
			}
			ch = ' ' + ((p[0] >> 2) & 0x3f);
			putchar(ch == ' ' ? '`' : ch);
			ch = ' ' + (((p[0] << 4) | ((p[1] >> 4) & 0xf)) & 0x3f);
			putchar(ch == ' ' ? '`' : ch);
			ch = ' ' + (((p[1] << 2) | ((p[2] >> 6) & 0x3)) & 0x3f);
			putchar(ch == ' ' ? '`' : ch);
			ch = ' ' + (p[2] & 0x3f);
			putchar(ch == ' ' ? '`' : ch);
		}
		putchar('\n');
	}
	if (ferror(fp))
		eprintf("'%s' read error:", s);
	fprintf(stdout, "%c\nend\n", '`');
}
