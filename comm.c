/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define CLAMP(x, l, h) MIN(h, MAX(l, x))

static int show = 0x07;

static void
printline(int pos, char *line)
{
	int i;

	if (!(show & (0x1 << pos)))
		return;

	for (i = 0; i < pos; i++) {
		if (show & (0x1 << i))
			putchar('\t');
	}
	printf("%s", line);
}

static char *
nextline(char *buf, int n, FILE *f, char *name)
{
	buf = fgets(buf, n, f);
	if (!buf && !feof(f))
		eprintf("%s: read error:", name);
	if (buf && !strchr(buf, '\n'))
		eprintf("%s: line too long\n", name);
	return buf;
}

static void
finish(int pos, FILE *f, char *name)
{
	char buf[LINE_MAX + 1];

	while (nextline(buf, sizeof(buf), f, name))
		printline(pos, buf);
	exit(1);
}

static void
usage(void)
{
	eprintf("usage: %s [-123] file1 file2\n", argv0);
}

int
main(int argc, char *argv[])
{
	int i, diff = 0;
	FILE *fp[2];
	char lines[2][LINE_MAX + 1];

	ARGBEGIN {
	case '1':
	case '2':
	case '3':
		show &= 0x07 ^ (1 << (ARGC() - '1'));
		break;
	default:
		usage();
	} ARGEND;

	if (argc != 2)
		usage();

	for (i = 0; i < LEN(fp); i++) {
		if (argv[i][0] == '-' && !argv[i][1])
			argv[i] = "/dev/fd/0";
		if (!(fp[i] = fopen(argv[i], "r")))
			eprintf("fopen %s:", argv[i]);
	}

	for (;;) {
		if (diff <= 0) {
			lines[0][0] = '\0';
			if (!nextline(lines[0], sizeof(lines[0]),
				     fp[0], argv[0])) {
				if (lines[1][0] != '\0')
					printline(1, lines[1]);
				finish(1, fp[1], argv[1]);
			}
		}
		if (diff >= 0) {
			lines[1][0] = '\0';
			if (!nextline(lines[1], sizeof(lines[1]),
				     fp[1], argv[1])) {
				if (lines[0][0] != '\0')
					printline(0, lines[0]);
				finish(0, fp[0], argv[0]);
			}
		}
		diff = strcmp(lines[0], lines[1]);
		diff = CLAMP(diff, -1, 1);
		printline((2-diff) % 3, lines[MAX(0, diff)]);
	}

	return 0;
}
