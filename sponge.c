/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: sponge file\n");
}

int
main(int argc, char *argv[])
{
	FILE *fp, *tmpfp;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc != 1)
		usage();

	if(!(tmpfp = tmpfile()))
		eprintf("tmpfile:");

	concat(stdin, "<stdin>", tmpfp, "<tmpfile>");
	rewind(tmpfp);

	if(!(fp = fopen(argv[0], "w")))
		eprintf("sponge: '%s':", argv[0]);
	concat(tmpfp, "<tmpfile>", fp, argv[0]);

	fclose(fp);
	fclose(tmpfp);

	return 0;
}
