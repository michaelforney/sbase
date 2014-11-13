/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "crypt.h"
#include "sha256.h"
#include "util.h"

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
		return cryptcheck(checkfile, argc, argv, &sha256_ops, md, sizeof(md));
	return cryptmain(argc, argv, &sha256_ops, md, sizeof(md));
}
