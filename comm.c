/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

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
	fputs(line, stdout);
}

static void
usage(void)
{
	eprintf("usage: %s [-123] file1 file2\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp[2];
	size_t linelen[2] = { 0, 0 };
	int ret = 0, i, diff = 0;
	char *line[2] = { NULL, NULL };

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

	for (i = 0; i < 2; i++) {
		if (!strcmp(argv[i], "-")) {
			argv[i] = "<stdin>";
			fp[i] = stdin;
		} else if (!(fp[i] = fopen(argv[i], "r"))) {
			eprintf("fopen %s:", argv[i]);
		}
	}

	for (;;) {
		for (i = 0; i < 2; i++) {
			if (diff && i == (diff < 0))
				continue;
			if (getline(&line[i], &linelen[i], fp[i]) > 0)
				continue;
			if (ferror(fp[i]))
				eprintf("getline %s:", argv[i]);
			if (diff && line[!i][0])
				printline(!i, line[!i]);
			while (getline(&line[!i], &linelen[!i], fp[!i]) > 0)
				printline(!i, line[!i]);
			if (ferror(fp[!i]))
				eprintf("getline %s:", argv[!i]);
			goto end;
		}
		diff = strcmp(line[0], line[1]);
		LIMIT(diff, -1, 1);
		printline((2 - diff) % 3, line[MAX(0, diff)]);
	}
end:
	ret |= fshut(fp[0], argv[0]);
	ret |= (fp[0] != fp[1]) && fshut(fp[1], argv[1]);
	ret |= fshut(stdout, "<stdout>");

	return ret;
}
