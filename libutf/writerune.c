/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utf.h"

void
writerune(const char *file, FILE *fp, Rune *r)
{
	char buf[UTFmax];
	int n;

	if ((n = runetochar(buf, r)) > 0) {
		fwrite(buf, n, 1, fp);
		if (ferror(fp)) {
			fprintf(stderr, "%s: write error: %s\n",
				file, strerror(errno));
			exit(1);
		}
	}
}
