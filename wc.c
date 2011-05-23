/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

static void output(const char *, long, long, long);
static void wc(FILE *, const char *);

static bool lflag = false;
static bool wflag = false;
static char cmode = 0;
static long tc = 0, tl = 0, tw = 0;

int
main(int argc, char *argv[])
{
	bool many;
	int i;
	FILE *fp;

	for(i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-c"))
			cmode = 'c';
		else if(!strcmp(argv[i], "-l"))
			lflag = true;
		else if(!strcmp(argv[i], "-m"))
			cmode = 'm';
		else if(!strcmp(argv[i], "-w"))
			wflag = true;
		else
			break;
	many = (argc > i+1);

	if(i == argc)
		wc(stdin, NULL);
	else for(; i < argc; i++) {
		if(!(fp = fopen(argv[i], "r")))
			eprintf("fopen %s:", argv[i]);
		wc(fp, argv[i]);
		fclose(fp);
	}
	if(many)
		output("total", tc, tl, tw);
	return EXIT_SUCCESS;
}

void
output(const char *str, long nc, long nl, long nw)
{
	bool noflags = !cmode && !lflag && !wflag;

	if(lflag || noflags)
		printf(" %5ld", nl);
	if(wflag || noflags)
		printf(" %5ld", nw);
	if(cmode || noflags)
		printf(" %5ld", nc);
	if(str)
		printf(" %s", str);
	fputc('\n', stdout);
}

void
wc(FILE *fp, const char *str)
{
	bool word = false;
	char c;
	long nc = 0, nl = 0, nw = 0;

	while((c = fgetc(fp)) != EOF) {
		if(cmode != 'm' || (c & 0xc0) != 0x80)
			nc++;
		if(c == '\n')
			nl++;
		if(!isspace(c))
			word = true;
		else if(word) {
			word = false;
			nw++;
		}
	}
	tc += nc;
	tl += nl;
	tw += nw;
	output(str, nc, nl, nw);
}
