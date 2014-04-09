#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../util.h"

void
parsemode(const char *str, mode_t *mode, int *oper)
{
	char *end;
	const char *p;
	int octal;
	mode_t mask = 0;

	octal = strtol(str, &end, 8);
	if(*end == '\0') {
		if(octal < 0 || octal > 07777)
			eprintf("%s: invalid mode\n", str);
		if(octal & 04000) *mode |= S_ISUID;
		if(octal & 02000) *mode |= S_ISGID;
		if(octal & 01000) *mode |= S_ISVTX;
		if(octal & 00400) *mode |= S_IRUSR;
		if(octal & 00200) *mode |= S_IWUSR;
		if(octal & 00100) *mode |= S_IXUSR;
		if(octal & 00040) *mode |= S_IRGRP;
		if(octal & 00020) *mode |= S_IWGRP;
		if(octal & 00010) *mode |= S_IXGRP;
		if(octal & 00004) *mode |= S_IROTH;
		if(octal & 00002) *mode |= S_IWOTH;
		if(octal & 00001) *mode |= S_IXOTH;
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
			if(oper)
				*oper = (int)*p;
			break;
		/* modes */
		case 'r':
			*mode |= S_IRUSR|S_IRGRP|S_IROTH;
			break;
		case 'w':
			*mode |= S_IWUSR|S_IWGRP|S_IWOTH;
			break;
		case 'x':
			*mode |= S_IXUSR|S_IXGRP|S_IXOTH;
			break;
		case 's':
			*mode |= S_ISUID|S_ISGID;
			break;
		case 't':
			*mode |= S_ISVTX;
			break;
			/* error */
		default:
			eprintf("%s: invalid mode\n", str);
		}
	if(mask)
		*mode &= mask;
}
