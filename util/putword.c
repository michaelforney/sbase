/* See LICENSE file for copyright and license details. */
#include <stdio.h>

#include "../util.h"

void
putword(const char *s)
{
	static int first = 1;

	if (!first)
		putchar(' ');

	fputs(s, stdout);
	first = 0;
}
