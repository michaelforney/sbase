/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void grep(FILE *, const char *, regex_t *);

static bool iflag = false;
static bool vflag = false;
static bool many;
static bool match = false;
static char mode = 0;

int
main(int argc, char *argv[])
{
	char c;
	int flags = 0;
	regex_t preg;
	FILE *fp;

	while((c = getopt(argc, argv, "cilnqv")) != -1)
		switch(c) {
		case 'c':
		case 'l':
		case 'n':
		case 'q':
			mode = c;
			break;
		case 'i':
			iflag = true;
			break;
		case 'v':
			vflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc) {
		fprintf(stderr, "usage: %s [-cilnqv] pattern [files...]\n", argv[0]);
		exit(2);
	}
	if(mode == 'c')
		flags |= REG_NOSUB;
	if(iflag)
		flags |= REG_ICASE;
	regcomp(&preg, argv[optind++], flags);

	many = (argc > optind+1);
	if(optind == argc)
		grep(stdin, "<stdin>", &preg);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r"))) {
			fprintf(stderr, "fopen %s: ", argv[optind]);
			perror(NULL);
			exit(2);
		}
		grep(fp, argv[optind], &preg);
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
