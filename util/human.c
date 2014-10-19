#include <stdio.h>
#include <string.h>

#include "../util.h"

char *
humansize(double n)
{
	static char buf[16];
	const char postfixes[] = "BKMGTPE";
	size_t i;

	for(i = 0; n >= 1024 && i < strlen(postfixes); i++)
		n /= 1024;

	if(!i)
		snprintf(buf, sizeof(buf), "%lu", (unsigned long)n);
	else
		snprintf(buf, sizeof(buf), "%.1f%c", n, postfixes[i]);
	return buf;
}
