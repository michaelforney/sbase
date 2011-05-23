/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static void cat(FILE *, const char *);

int
main(int argc, char *argv[])
{
	int i;
	FILE *fp;

	if(argc == 1)
		cat(stdin, "<stdin>");
	else for(i = 1; i < argc; i++) {
		if(!(fp = fopen(argv[i], "r")))
			eprintf("fopen %s:", argv[i]);
		cat(fp, argv[i]);
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
