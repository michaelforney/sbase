/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../text.h"
#include "../util.h"

void
getlines(FILE *fp, struct linebuf *b)
{
	char *line = NULL;
	size_t size = 0, linelen;
	ssize_t len;

	while ((len = getline(&line, &size, fp)) > 0) {
		if (++b->nlines > b->capacity) {
			b->capacity += 512;
			b->lines = erealloc(b->lines, b->capacity * sizeof(*b->lines));
		}
		linelen = len + 1;
		b->lines[b->nlines - 1] = memcpy(emalloc(linelen), line, linelen);
	}
	free(line);
	if (b->lines && b->nlines && !strchr(b->lines[b->nlines - 1], '\n')) {
		b->lines[b->nlines - 1] = erealloc(b->lines[b->nlines - 1], linelen + 1);
		b->lines[b->nlines - 1][linelen - 1] = '\n';
		b->lines[b->nlines - 1][linelen] = '\0';
	}
}
