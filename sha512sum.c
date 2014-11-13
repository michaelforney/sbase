/* See LICENSE file for copyright and license details. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "crypt.h"
#include "sha512.h"
#include "util.h"

struct sha512 s;
struct crypt_ops sha512_ops = {
	sha512_init,
	sha512_update,
	sha512_sum,
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
	uint8_t md[SHA512_DIGEST_LENGTH];
	char *checkfile = NULL;
	bool cflag = false;

	ARGBEGIN {
	case 'c':
		cflag = true;
		checkfile = ARGF();
		break;
	default:
		usage();
	} ARGEND;

	if (cflag)
		return cryptcheck(checkfile, argc, argv, &sha512_ops, md, sizeof(md));
	return cryptmain(argc, argv, &sha512_ops, md, sizeof(md));
}
