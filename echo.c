/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	bool nflag = false;
	char c;

	while((c = getopt(argc, argv, "n")) != -1)
		switch(c) {
		case 'n':
			nflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	for(; optind < argc; optind++) {
		fputs(argv[optind], stdout);
		if(optind+1 < argc)
			fputc(' ', stdout);
	}
	if(!nflag)
		fputc('\n', stdout);
	return EXIT_SUCCESS;
}
