/* See LICENSE file for copyright and license details. */
#include "../utf.h"

int
utftorunestr(const char *str, Rune *r)
{
	int i, n;

	for(i = 0; (n = chartorune(&r[i], str)) && r[i]; i++)
		str += n;

	return i;
}
