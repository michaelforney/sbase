/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <string.h>

#include "../util.h"

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	p = calloc(nmemb, size);
	if (!p)
		eprintf("calloc: out of memory\n");
	return p;
}

void *
emalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (!p)
		eprintf("malloc: out of memory\n");
	return p;
}

void *
erealloc(void *p, size_t size)
{
	p = realloc(p, size);
	if (!p)
		eprintf("realloc: out of memory\n");
	return p;
}

char *
estrdup(const char *s)
{
	char *p;

	p = strdup(s);
	if (!p)
		eprintf("strdup: out of memory\n");
	return p;
}
