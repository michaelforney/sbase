/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../util.h"

char *
humansize(size_t n)
{
	static char buf[16];
	const char postfixes[] = "BKMGTPE";
	double size;
	int i;

	for (size = n, i = 0; size >= 1024 && i < strlen(postfixes); i++)
		size /= 1024;

	if (!i)
		snprintf(buf, sizeof(buf), "%zu", n);
	else
		snprintf(buf, sizeof(buf), "%.1f%c", size, postfixes[i]);

	return buf;
}
