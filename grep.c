/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void grep(FILE *, const char *, regex_t *);

static bool iflag = false;
static bool vflag = false;
static bool many;
static bool match = false;
static char mode = 0;

int
main(int argc, char *argv[])
{
	int i, flags = 0;
	regex_t preg;
	FILE *fp;

	for(i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-c"))
			mode = 'c';
		else if(!strcmp(argv[i], "-i"))
			iflag = true;
		else if(!strcmp(argv[i], "-l"))
			mode = 'l';
		else if(!strcmp(argv[i], "-n"))
			mode = 'n';
		else if(!strcmp(argv[i], "-q"))
			mode = 'q';
		else if(!strcmp(argv[i], "-v"))
			vflag = true;
		else
			break;

	if(i == argc) {
		fprintf(stderr, "usage: %s [-c] [-i] [-l] [-n] [-v] pattern [files...]\n", argv[0]);
		exit(2);
	}
	if(mode == 'c')
		flags |= REG_NOSUB;
	if(iflag)
		flags |= REG_ICASE;
	regcomp(&preg, argv[i++], flags);

	many = (argc > i+1);
	if(i == argc)
		grep(stdin, "<stdin>", &preg);
	else for(; i < argc; i++) {
		if(!(fp = fopen(argv[i], "r"))) {
			fprintf(stderr, "fopen %s: ", argv[i]);
			perror(NULL);
			exit(2);
		}
		grep(fp, argv[i], &preg);
		fclose(fp);
	}
	return match ? 0 : 1;
}

void
grep(FILE *fp, const char *str, regex_t *preg)
{
	char buf[BUFSIZ];
	int n, c = 0;

	for(n = 1; fgets(buf, sizeof buf, fp); n++) {
		if(regexec(preg, buf, 0, NULL, 0) ^ vflag)
			continue;
		if(mode == 'c')
			c++;
		else if(mode == 'l') {
			puts(str);
			break;
		}
		else if(mode == 'q')
			exit(0);
		else {
			if(many)
				printf("%s:", str);
			if(mode == 'n')
				printf("%d:", n);
			fputs(buf, stdout);
		}
		match = true;
	}
	if(mode == 'c')
		printf("%d\n", c);
	if(ferror(fp)) {
		fprintf(stderr, "%s: read error: ", str);
		perror(NULL);
		exit(2);
	}
}
