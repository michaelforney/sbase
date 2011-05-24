/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	char *s;
	size_t n;

	if(argc < 2)
		eprintf("usage: %s string [suffix]\n", argv[0]);

	s = basename(argv[1]);
	if(argc > 2 && strlen(s) > strlen(argv[2])) {
		n = strlen(s) - strlen(argv[2]);
		if(!strcmp(&s[n], argv[2]))
			s[n] = '\0';
	}
	puts(s);
	return EXIT_SUCCESS;
}
