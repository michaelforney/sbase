/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

static void chmodr(const char *);
static void parsemode(const char *);

static bool rflag = false;
static char oper = '=';
static mode_t mode = 0;

int
main(int argc, char *argv[])
{
	char c;

	while((c = getopt(argc, argv, "r")) != -1)
		switch(c) {
		case 'r':
			rflag = true;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	if(optind == argc)
		eprintf("usage: %s [-r] mode [file...]\n", argv[0]);

	parsemode(argv[optind++]);
	for(; optind < argc; optind++)
		chmodr(argv[optind]);
	return EXIT_SUCCESS;
}

void
chmodr(const char *path)
{
	struct stat st;

	if(stat(path, &st) == -1)
		eprintf("stat %s:", path);

	switch(oper) {
	case '+':
		st.st_mode |= mode;
		break;
	case '-':
		st.st_mode &= ~mode;
		break;
	case '=':
		st.st_mode = mode;
		break;
	}
	if(chmod(path, st.st_mode) == -1)
		eprintf("chmod %s:", path);
	if(rflag)
		recurse(path, chmodr);
}

void
parsemode(const char *str)
{
	char *end;
	const char *p;
	int octal;
	mode_t mask = 0;

	octal = strtol(str, &end, 8);
	if(*end == '\0') {
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
		return;
	}
	for(p = str; *p; p++)
		switch(*p) {
		/* masks */
		case 'u':
			mask |= S_IRWXU;
			break;
		case 'g':
			mask |= S_IRWXG;
			break;
		case 'o':
			mask |= S_IRWXO;
			break;
		case 'a':
			mask |= S_IRWXU|S_IRWXG|S_IRWXO;
			break;
		/* opers */
		case '+':
		case '-':
		case '=':
			oper = *p;
			break;
		/* modes */
		case 'r':
			mode |= S_IRUSR|S_IRGRP|S_IROTH;
			break;
		case 'w':
			mode |= S_IWUSR|S_IWGRP|S_IWOTH;
			break;
		case 'x':
			mode |= S_IXUSR|S_IXGRP|S_IXOTH;
			break;
		case 's':
			mode |= S_ISUID|S_ISGID;
			break;
		/* error */
		default:
			eprintf("%s: invalid mode\n", str);
		}
	if(mask)
		mode &= mask;
}
