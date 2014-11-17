/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <unistd.h>

#include "../text.h"
#include "../util.h"

void
concat(FILE *fp1, const char *s1, FILE *fp2, const char *s2)
{
	char buf[BUFSIZ];
	ssize_t n;

	while ((n = read(fileno(fp1), buf, sizeof buf)) > 0) {
		if (write(fileno(fp2), buf, n) != n)
			eprintf("%s: write error:", s2);
	}
	if (n < 0)
		eprintf("%s: read error:", s1);
}
