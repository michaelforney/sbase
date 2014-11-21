/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utf.h"

void
writerune(Rune *r)
{
	char buf[UTFmax];
	int n;

	if ((n = runetochar(buf, r)) > 0) {
		fwrite(buf, n, 1, stdout);
		if (ferror(stdout)) {
			fprintf(stderr, "stdout: write error: %s\n",
				strerror(errno));
			exit(1);
		}
	}
}
