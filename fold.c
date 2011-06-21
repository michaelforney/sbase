/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

static void fold(FILE *, long);
static void foldline(const char *, long);

static bool bflag = false;
static bool sflag = false;

int
main(int argc, char *argv[])
{
	char c;
	long width = 80;
	FILE *fp;

	while((c = getopt(argc, argv, "bsw:")) != -1)
		switch(c) {
		case 'b':
			bflag = true;
			break;
		case 's':
			sflag = true;
			break;
		case 'w':
			width = estrtol(optarg, 0);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc)
		fold(stdin, width);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		fold(fp, width);
		fclose(fp);
	}
	return EXIT_SUCCESS;
}

void
fold(FILE *fp, long width)
{
	char *buf = NULL;
	size_t size = 0;

	while(afgets(&buf, &size, fp))
		foldline(buf, width);
	free(buf);
}

void
foldline(const char *str, long width)
{
	bool space;
	long col, j;
	size_t i = 0, n = 0;

	do {
		space = false;
		for(j = i, col = 0; str[j] && col <= width; j++) {
			if(!UTF8_POINT(str[j]) && !bflag)
				continue;
			if(sflag && isspace(str[j])) {
				space = true;
				n = j+1;
			}
			else if(!space)
				n = j;

			if(!bflag && iscntrl(str[j]))
				switch(str[j]) {
				case '\b':
					col--;
					break;
				case '\r':
					col = 0;
					break;
				case '\t':
					col += (col+1) % 8;
					break;
				}
			else
				col++;
		}
		if(fwrite(&str[i], 1, n-i, stdout) != n-i)
			eprintf("<stdout>: write error:");
		if(str[n])
			putchar('\n');
	} while(str[i = n] && str[i] != '\n');
}
