/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <string.h>

#include "../util.h"
#include "../utf.h"

int
chartorunearr(const char *str, Rune **r)
{
	size_t len = strlen(str), rlen, roff, ret, i;
	Rune s;

	for (rlen = 0, roff = 0; roff < len && ret; rlen++) {
		ret = charntorune(&s, str + roff, MAX(UTFmax, len - roff));
		roff += ret;
	}

	*r = emalloc(rlen * sizeof(Rune) + 1);
	(*r)[rlen] = 0;

	for (i = 0, roff = 0; roff < len && i < rlen; i++) {
		roff += charntorune(&(*r)[i], str + roff, MAX(UTFmax, len - roff));
	}

	return rlen;
}
