/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	bool nflag = false;
	int i;

	if(argc > 1 && !strcmp(argv[1], "-n"))
		nflag = true;
	for(i = nflag ? 2 : 1; i < argc; i++) {
		fputs(argv[i], stdout);
		if(i+1 < argc)
			fputc(' ', stdout);
	}
	if(!nflag)
		fputc('\n', stdout);
	return EXIT_SUCCESS;
}
