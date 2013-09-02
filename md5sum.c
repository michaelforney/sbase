/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"
#include "crypt.h"
#include "md5.h"

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

	ARGBEGIN {
	case 'c':
		eprintf("not implemented\n");
	default:
		usage();
	} ARGEND;

	return cryptmain(argc, argv, &md5_ops, md, sizeof(md));
}
