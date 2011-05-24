/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	char *s;
	size_t n;

	if(getopt(argc, argv, "") != -1)
		exit(EXIT_FAILURE);
	if(optind == argc)
		eprintf("usage: %s string [suffix]\n", argv[0]);

	s = basename(argv[optind++]);
	if(optind < argc && strlen(s) > strlen(argv[optind])) {
		n = strlen(s) - strlen(argv[optind]);
		if(!strcmp(&s[n], argv[optind]))
			s[n] = '\0';
	}
	puts(s);
	return EXIT_SUCCESS;
}
