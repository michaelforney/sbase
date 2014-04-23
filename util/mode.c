#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../util.h"

mode_t
getumask(void)
{
    mode_t mask = umask(0);
    umask(mask);
    return mask;
}

mode_t
parsemode(const char *str, mode_t mode, mode_t mask)
{
	char *end;
	const char *p;
	int octal, op = '+';
	mode_t gmask = 0, m = 0;

	octal = strtol(str, &end, 8);
	if(*end == '\0') {
		if(octal < 0 || octal > 07777) {
			eprintf("%s: invalid mode\n", str);
			return -1;
		}
		mode = 0;
		if(octal & 04000) mode |= S_ISUID;
		if(octal & 02000) mode |= S_ISGID;
		if(octal & 01000) mode |= S_ISVTX;
		if(octal & 00400) mode |= S_IRUSR;
		if(octal & 00200) mode |= S_IWUSR;
		if(octal & 00100) mode |= S_IXUSR;
		if(octal & 00040) mode |= S_IRGRP;
		if(octal & 00020) mode |= S_IWGRP;
		if(octal & 00010) mode |= S_IXGRP;
		if(octal & 00004) mode |= S_IROTH;
		if(octal & 00002) mode |= S_IWOTH;
		if(octal & 00001) mode |= S_IXOTH;
		return mode;
	}
	for(p = str; *p; p++) {
		switch(*p) {
		/* masks */
		case 'u':
			gmask |= S_IRWXU;
			break;
		case 'g':
			gmask |= S_IRWXG;
			break;
		case 'o':
			gmask |= S_IRWXO;
			break;
		case 'a':
			gmask |= S_IRWXU|S_IRWXG|S_IRWXO;
			break;
		/* opers */
		case '=':
		case '+':
		case '-':
			op = (int)*p;
			break;
		/* modes */
		case 'r':
			m |= S_IRUSR|S_IRGRP|S_IROTH;
			break;
		case 'w':
			m |= S_IWUSR|S_IWGRP|S_IWOTH;
			break;
		case 'x':
			m |= S_IXUSR|S_IXGRP|S_IXOTH;
			break;
		case 's':
			m |= S_ISUID|S_ISGID;
			break;
		case 't':
			m |= S_ISVTX;
			break;
		default:
			eprintf("%s: invalid mode\n", str);
			return -1;
		}
		/* apply */
		switch(op) {
		case '+':
			mode |= (m & ((~mask) & 0777));
			break;
		case '-':
			mode &= (~(m & ((~mask) & 0777)));
			break;
		case '=':
			mode = (m & ((~mask) & 0777));
			break;
		}
	}
	if(gmask && op != '=')
		mode &= ~gmask;
	return mode;
}
