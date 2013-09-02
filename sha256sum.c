/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"
#include "crypt.h"
#include "sha256.h"

struct sha256 s;
struct crypt_ops sha256_ops = {
	sha256_init,
	sha256_update,
	sha256_sum,
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
	uint8_t md[SHA256_DIGEST_LENGTH];

	ARGBEGIN {
	case 'c':
		eprintf("not implemented\n");
	default:
		usage();
	} ARGEND;

	return cryptmain(argc, argv, &sha256_ops, md, sizeof(md));
}
