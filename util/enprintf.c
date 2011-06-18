/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include "../util.h"

extern void venprintf(int, const char *, va_list);

void
enprintf(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	venprintf(status, fmt, ap);
	va_end(ap);
}
