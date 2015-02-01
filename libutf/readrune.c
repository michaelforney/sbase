/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utf.h"

int
readrune(const char *file, FILE *fp, Rune *r)
{
	char buf[UTFmax];
	int c, i;

	if ((c = fgetc(fp)) == EOF) {
		if (ferror(fp)) {
			fprintf(stderr, "%s: read error: %s\n",
				file, strerror(errno));
			exit(1);
		}
		return 0;
	}

	if (c < Runeself) {
		*r = (Rune)c;
		return 1;
	}

	buf[0] = c;
	for (i = 1; ;) {
		if ((c = fgetc(fp)) == EOF) {
			if (ferror(fp)) {
				fprintf(stderr, "%s: read error: %s\n",
					file, strerror(errno));
				exit(1);
			}
			*r = Runeerror;
			return i;
		}
		buf[i++] = c;
		if (fullrune(buf, i)) {
			chartorune(r, buf);
			break;
		}
	}
	return i;
}
