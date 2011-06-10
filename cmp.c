/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

static bool lflag = false;
static bool sflag = false;

int
main(int argc, char *argv[])
{
	bool same = true;
	char c;
	int b[2], i;
	long line = 1, n = 1;
	FILE *fp[2];

	if((c = getopt(argc, argv, "ls")) != -1)
		switch(c) {
		case 'l':
			lflag = true;
			break;
		case 's':
			sflag = true;
			break;
		default:
			exit(2);
		}
	if(optind != argc-2) {
		eprintf("usage: %s [-ls] file1 file2\n", argv[0]);
		exit(2);
	}
	for(i = 0; i < 2; i++)
		if(!(fp[i] = fopen(argv[optind+i], "r"))) {
			eprintf("fopen %s:", argv[optind+i]);
			exit(2);
		}
	for(n = 1; ((b[0] = getc(fp[0])) != EOF) | ((b[1] = getc(fp[1])) != EOF); n++) {
		if(b[0] == '\n')
			line++;
		if(b[0] == b[1])
			continue;
		for(i = 0; i < 2; i++)
			if(b[i] == EOF) {
				fprintf(stderr, "cmp: EOF on %s\n", argv[optind+i]);
				return 1;
			}
		if(!lflag) {
			if(!sflag)
				printf("%s %s differ: char %ld, line %ld\n",
				       argv[optind], argv[optind+1], n, line);
			return 1;
		}
		else {
			printf("%4ld %3o %3o\n", n, b[0], b[1]);
			same = false;
		}
	}
	return same ? 0 : 1;
}
