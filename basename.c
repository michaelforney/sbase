/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define USAGE()  usage("name [suffix]")

int
main(int argc, char *argv[])
{
	char *s;
	size_t n;

	ARGBEGIN {
	default:
		USAGE();
	} ARGEND;

	if(argc < 1)
		USAGE();

	s = basename(argv[0]);
	if(argc == 2 && argv[1]) {
		n = strlen(s) - strlen(argv[1]);
		if(!strcmp(&s[n], argv[1]))
			s[n] = '\0';
	}
	puts(s);

	return EXIT_SUCCESS;
}
