/* See LICENSE file for copyright and license details. */
#include "../utf.h"

size_t
utftorunestr(const char *str, Rune *r)
{
	size_t i, n;

	for (i = 0; (n = chartorune(&r[i], str)) && r[i]; i++)
		str += n;

	return i;
}
