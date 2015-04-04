/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "../text.h"
#include "../util.h"

void
concat(FILE *fp1, const char *s1, FILE *fp2, const char *s2)
{
	char buf[BUFSIZ];
	size_t n;

	while ((n = fread(buf, 1, sizeof(buf), fp1))) {
		fwrite(buf, 1, n, fp2);

		if (feof(fp1) || ferror(fp1) || ferror(fp2))
			break;
	}
}
