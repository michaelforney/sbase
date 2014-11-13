/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "crypt.h"
#include "md5.h"
#include "util.h"

struct md5 s;
struct crypt_ops md5_ops = {
	md5_init,
	md5_update,
	md5_sum,
	&s,
};

static void
usage(void)
{
	eprintf("usage: %s [-c] [file...]\n", argv0);
}

int
main(int argc, char *argv[])
{
	uint8_t md[MD5_DIGEST_LENGTH];
	char *checkfile = NULL;
	int cflag = 0;

	ARGBEGIN {
	case 'c':
		cflag = 1;
		checkfile = ARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (cflag)
		return cryptcheck(checkfile, argc, argv, &md5_ops, md, sizeof(md));
	return cryptmain(argc, argv, &md5_ops, md, sizeof(md));
}
