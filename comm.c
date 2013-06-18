/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

enum { Suppress1 = 1, Suppress2 = 2, Suppress3 = 4 };

static void comm(FILE *fp1, const char *s1, FILE *fp2,
		 const char *s2, int sflags);

static void
usage(void)
{
	eprintf("usage: %s [-123] file1 file2\n", argv0);
}

int
main(int argc, char *argv[])
{
	int sflags = 0;
	FILE *fp1, *fp2;

	ARGBEGIN {
	case '1':
		sflags |= Suppress1;
		break;
	case '2':
		sflags |= Suppress2;
		break;
	case '3':
		sflags |= Suppress3;
		break;
	default:
		usage();
	} ARGEND;

	if (argc < 2)
		usage();

	if (!(fp1 = fopen(argv[0], "r")))
		eprintf("fopen %s:", argv[0]);
	if (!(fp2 = fopen(argv[1], "r")))
		eprintf("fopen %s:", argv[1]);

	comm(fp1, argv[0], fp2, argv[1], sflags);

	return 0;
}

static void
print_col1(const char *s, int sflags)
{
	if (sflags & Suppress1)
		return;
	printf("%s", s);
}

static void
print_col2(const char *s, int sflags)
{
	const char *tabs = "\t";
	if (sflags & Suppress1)
		tabs = "";
	if (sflags & Suppress2)
		return;
	printf("%s%s", tabs, s);
}

static void
print_col3(const char *s, int sflags)
{
	const char *tabs = "\t\t";
	if (sflags & Suppress1)
		tabs = "\t";
	if (sflags & Suppress2)
		tabs = "";
	if (sflags & Suppress3)
		return;
	printf("%s%s", tabs, s);
}

static void
comm(FILE *fp1, const char *s1, FILE *fp2, const char *s2, int sflags)
{
	char buf1[BUFSIZ], buf2[BUFSIZ];
	bool eof1 = false, eof2 = false;
	bool r1 = true, r2 = true;
	int ret;

	for (;;) {
		if (r1)
			if (!fgets(buf1, sizeof buf1, fp1))
				eof1 = true;
		if (r2)
			if (!fgets(buf2, sizeof buf2, fp2))
				eof2 = true;

		/* If we reached EOF on fp1 then just dump fp2 */
		if (eof1) {
			do {
				print_col2(buf2, sflags);
			} while (fgets(buf2, sizeof buf2, fp2));
			return;
		}
		/* If we reached EOF on fp2 then just dump fp1 */
		if (eof2) {
			do {
				print_col1(buf1, sflags);
			} while (fgets(buf1, sizeof buf1, fp1));
			return;
		}

		ret = strcmp(buf1, buf2);
		if (!ret) {
			r1 = r2 = true;
			print_col3(buf1, sflags);
			continue;
		} else if (ret < 0) {
			r1 = true;
			r2 = false;
			print_col1(buf1, sflags);
			continue;
		} else {
			r1 = false;
			r2 = true;
			print_col2(buf2, sflags);
			continue;
		}
	}
}
