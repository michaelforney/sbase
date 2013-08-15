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
	size_t size = 0;
	size_t linelen;

	while(afgets(&line, &size, fp)) {
		if(++b->nlines > b->capacity) {
			b->capacity += 512;
			nline = realloc(b->lines, b->capacity * sizeof(*b->lines));
			if(nline == NULL)
				eprintf("realloc:");
			b->lines = nline;
		}
		if(!(b->lines[b->nlines-1] = malloc((linelen = strlen(line)+1))))
			eprintf("malloc:");
		memcpy(b->lines[b->nlines-1], line, linelen);
	}
	free(line);
}

