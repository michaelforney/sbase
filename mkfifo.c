/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

int
main(int argc, char *argv[])
{
	while(getopt(argc, argv, "") != -1)
		exit(EXIT_FAILURE);
	for(; optind < argc; optind++)
		if(mkfifo(argv[optind], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) == -1)
			eprintf("mkfifo %s:", argv[optind]);
	return EXIT_SUCCESS;
}
