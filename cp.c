/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fs.h"
#include "util.h"

int
main(int argc, char *argv[])
{
	struct stat st;
	char c;
	
	while((c = getopt(argc, argv, "r")) != -1)
		switch(c) {
		case 'r':
			cp_rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(argc > 3 && !cp_rflag && !(stat(argv[argc-1], &st) == 0 && S_ISDIR(st.st_mode)))
		eprintf("%s: not a directory\n", argv[argc-1]);
	enmasse(argc - optind, &argv[optind], cp);
	return EXIT_SUCCESS;
}
