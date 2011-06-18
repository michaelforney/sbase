/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../util.h"

void
venprintf(int status, const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);

	if(fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	}
	exit(status);
}
