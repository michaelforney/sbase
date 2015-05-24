/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "text.h"
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s file\n", argv0);
}

int
main(int argc, char *argv[])
{
	FILE *fp, *tmpfp;
	int ret = 0;

	argv0 = argv[0], argc--, argv++;

	if (argc != 1)
		usage();

	if (!(tmpfp = tmpfile()))
		eprintf("tmpfile:");
	concat(stdin, "<stdin>", tmpfp, "<tmpfile>");
	rewind(tmpfp);

	if (!(fp = fopen(argv[0], "w")))
		eprintf("fopen %s:", argv[0]);
	concat(tmpfp, "<tmpfile>", fp, argv[0]);

	ret |= fshut(fp, argv[0]) | fshut(tmpfp, "<tmpfile>");

	return ret;
}
