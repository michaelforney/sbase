/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <ctype.h>

#include "../util.h"

int
confirm(const char *fmt, ...)
{
	int c, ans;
	va_list ap;

	va_start(ap, fmt);
	xvprintf(fmt, ap);
	va_end(ap);

	c = getchar();
	ans = (c == 'y' || c == 'Y');
	while (c != '\n' && c != EOF)
		c = getchar();
	return ans;
}
