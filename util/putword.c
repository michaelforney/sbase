/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include "../util.h"

void
putword(const char *s)
{
	static bool first = true;

	if(!first)
		putchar(' ');
	fputs(s, stdout);
	first = false;
}
