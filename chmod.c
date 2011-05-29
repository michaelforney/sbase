/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static void chmodr(const char *);

static bool rflag = false;
static mode_t mode = 0;

int
main(int argc, char *argv[])
{
	char c, *end;
	int octal;

	while((c = getopt(argc, argv, "Rr")) != -1)
		switch(c) {
		case 'R':
		case 'r':
			rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc)
		eprintf("usage: %s [-Rr] mode [file...]\n", argv[0]);
	octal = strtol(argv[optind++], &end, 8);
	if(*end != '\0')
		eprintf("%s: not an octal number\n", argv[optind-1]);

	/* posix doesn't specify modal bits */
	if(octal & 04000) mode |= S_ISUID;
	if(octal & 02000) mode |= S_ISGID;
	if(octal & 00400) mode |= S_IRUSR;
	if(octal & 00200) mode |= S_IWUSR;
	if(octal & 00100) mode |= S_IXUSR;
	if(octal & 00040) mode |= S_IRGRP;
	if(octal & 00020) mode |= S_IWGRP;
	if(octal & 00010) mode |= S_IXGRP;
	if(octal & 00004) mode |= S_IROTH;
	if(octal & 00002) mode |= S_IWOTH;
	if(octal & 00001) mode |= S_IXOTH;

	for(; optind < argc; optind++)
		chmodr(argv[optind]);
	return EXIT_SUCCESS;
}

void
chmodr(const char *path)
{
	if(chmod(path, mode) != 0)
		eprintf("chmod %s:", path);
	if(rflag)
		recurse(path, chmodr);
}
