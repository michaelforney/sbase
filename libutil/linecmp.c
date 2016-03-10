/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "../text.h"
#include "../util.h"

int
linecmp(struct line *a, struct line *b)
{
	int res = 0;

	if (!(res = memcmp(a->data, b->data, MIN(a->len, b->len)))) {
		if (a->len > b->len) {
			res = a->data[b->len];
		} else if (b->len > a->len) {
			res = -b->data[a->len];
		} else {
			res = 0;
		}
	}

	return res;
}
