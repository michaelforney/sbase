/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	char *str = argv[1];
	size_t n, i = 0;

	if(argc < 2)
		eprintf("usage: %s string [suffix]\n", argv[0]);
	if(str[0] != '\0')
		for(i = strlen(str)-1; i > 0 && str[i] == '/'; i--)
			str[i] = '\0';
	if(i == 0 || !(str = strrchr(argv[1], '/')))
		str = argv[1];
	else
		str++;

	if(argc > 2 && strlen(str) > strlen(argv[2])) {
		n = strlen(str) - strlen(argv[2]);
		if(!strcmp(&str[n], argv[2]))
			str[n] = '\0';
	}
	puts(str);
	return EXIT_SUCCESS;
}
