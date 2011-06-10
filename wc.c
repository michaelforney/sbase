/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
	char c;
	FILE *fp;

	while((c = getopt(argc, argv, "clmw")) != -1)
		switch(c) {
		case 'c':
		case 'm':
			cmode = c;
			break;
		case 'l':
			lflag = true;
			break;
		case 'w':
			wflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	many = (argc > optind+1);

	if(optind == argc)
		wc(stdin, NULL);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		wc(fp, argv[optind]);
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
	putchar('\n');
}

void
wc(FILE *fp, const char *str)
{
	bool word = false;
	char c;
	long nc = 0, nl = 0, nw = 0;

	while((c = getc(fp)) != EOF) {
		if(cmode != 'm' || UTF8_POINT(c))
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
