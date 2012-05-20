/* See LICENSE file for copyright and license details. */
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

static void nl(FILE *);

static char mode = 't';
static const char *sep = "\t";
static long incr = 1;
static regex_t preg;

int
main(int argc, char *argv[])
{
	char c;
	FILE *fp;

	while((c = getopt(argc, argv, "b:i:s:")) != -1)
		switch(c) {
		case 'b':
			mode = optarg[0];
			if(optarg[0] == 'p')
				regcomp(&preg, &optarg[1], REG_NOSUB);
			else if(!strchr("ant", optarg[0]) || optarg[1] != '\0')
				eprintf("usage: %s [-b mode] [-i increment] [-s separator] [file...]\n", argv[0]);
			break;
		case 'i':
			incr = estrtol(optarg, 0);
			break;
		case 's':
			sep = optarg;
			break;
		default:
			exit(2);
		}
	if(optind == argc)
		nl(stdin);
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		nl(fp);
		fclose(fp);
	}
	return EXIT_SUCCESS;
}

void
nl(FILE *fp)
{
	char *buf = NULL;
	long n = 0;
	size_t size = 0;

	while(afgets(&buf, &size, fp))
		if((mode == 'a')
		|| (mode == 'p' && !regexec(&preg, buf, 0, NULL, 0))
		|| (mode == 't' && buf[0] != '\n'))
			printf("%6ld%s%s", n += incr, sep, buf);
		else
			printf("       %s", buf);
	free(buf);
}
