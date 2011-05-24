/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void cat(FILE *, const char *);

int
main(int argc, char *argv[])
{
	FILE *fp;

	if(getopt(argc, argv, "") != -1)
		exit(EXIT_FAILURE);
	if(optind == argc)
		cat(stdin, "<stdin>");
	else for(; optind < argc; optind++) {
		if(!(fp = fopen(argv[optind], "r")))
			eprintf("fopen %s:", argv[optind]);
		cat(fp, argv[optind]);
		fclose(fp);
	}
	return EXIT_SUCCESS;
}

void
cat(FILE *fp, const char *str)
{
	char buf[BUFSIZ];
	size_t n;

	while((n = fread(buf, 1, sizeof buf, fp)) > 0)
		if(fwrite(buf, 1, n, stdout) != n)
			eprintf("<stdout>: write error:");
	if(ferror(fp))
		eprintf("%s: read error:", str);
}
