/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../text.h"
#include "../util.h"

void
getlines(FILE *fp, struct linebuf *b)
{
	char *line = NULL, **nline;
	size_t size = 0, linelen;
	ssize_t len;

	while ((len = agetline(&line, &size, fp)) != -1) {
		if (++b->nlines > b->capacity) {
			b->capacity += 512;
			nline = erealloc(b->lines, b->capacity * sizeof(*b->lines));
			b->lines = nline;
		}
		linelen = len + 1;
		b->lines[b->nlines-1] = emalloc(linelen);
		memcpy(b->lines[b->nlines-1], line, linelen);
	}
	free(line);
}
